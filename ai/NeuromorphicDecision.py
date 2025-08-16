import numpy as np
from lava.magma.core.run_configs import Loihi2HwCfg
from lava.magma.core.run_conditions import RunSteps
from lava.proc.dense.process import Dense
from lava.proc.lif.process import LIF
from lava.proc.io.source import RingBuffer
from lava.proc.io.sink import Sink

class NeuromorphicDecisionModule:
    def __init__(self, weights_path="quantum_trained_weights.npy"):
        # 加载预训练权重
        self.weights = np.load(weights_path)
        self.input_size = self.weights.shape[0]
        self.output_size = self.weights.shape[1]

        # 初始化Lava框架组件
        self._init_lava_components()

    def _init_lava_components(self):
        # 创建脉冲神经网络组件
        self.input = RingBuffer(shape=(self.input_size,))
        self.dense = Dense(weights=self.weights)
        self.lif = LIF(threshold=0.8, decay=0.95)
        self.output = Sink(shape=(self.output_size,))

        # 连接组件
        self.input.s_out.connect(self.dense.s_in)
        self.dense.a_out.connect(self.lif.a_in)
        self.lif.s_out.connect(self.output.s_in)

        # 设置运行配置
        self.run_config = Loihi2HwCfg()

    def encode_to_spikes(self, market_data):
        """将市场数据编码为脉冲序列"""
        # 简单的速率编码
        normalized_data = (market_data - np.min(market_data)) / (
            np.max(market_data) - np.min(market_data) + 1e-8)
        spike_probability = normalized_data * 0.5  # 最大50%的脉冲概率
        return (np.random.rand(*market_data.shape) < spike_probability).astype(int)

    def decode_spikes(self, spikes):
        """将脉冲序列解码为交易动作"""
        # 计算脉冲频率
        spike_counts = np.sum(spikes, axis=0)
        action = np.argmax(spike_counts)
        return action

    def make_decision(self, market_data):
        """基于市场数据做出交易决策"""
        # 数据编码
        spikes = self.encode_to_spikes(market_data)

        # 设置输入数据
        self.input.data = spikes

        # 运行网络
        self.lif.run(RunSteps(num_steps=spikes.shape[0]), run_cfg=self.run_config)

        # 获取输出
        output_spikes = self.output.data

        # 解码决策
        action = self.decode_spikes(output_spikes)
        return action