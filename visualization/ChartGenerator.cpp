#include "ChartGenerator.h"
#include <sstream>
#include <cmath>

namespace hft {
namespace visualization {

std::string ChartGenerator::generatePnLChart(
    const std::vector<double>& equity,
    const std::vector<std::string>& dates) {
        
    std::stringstream js;
    js << R"(
        const data = {
            type: 'scatter',
            mode: 'lines',
            x: [)" << vectorToJSArray(dates) << R"(],
            y: [)" << vectorToJSArray(equity) << R"(],
            name: 'Equity Curve',
            line: {
                color: '#17BECF',
                width: 2
            }
        };
        
        const layout = {
            title: 'Portfolio Performance',
            xaxis: {
                title: 'Date',
                showgrid: true,
                zeroline: false
            },
            yaxis: {
                title: 'Equity',
                showgrid: true,
                zeroline: true
            },
            plot_bgcolor: '#E5ECF6',
            showlegend: true
        };
        
        Plotly.newPlot('pnlChart', [data], layout, {responsive: true});
    )";
    
    return js.str();
}

std::string ChartGenerator::generateVolatilitySurface(
    const std::vector<std::vector<double>>& surface,
    const std::vector<double>& strikes,
    const std::vector<double>& maturities) {
        
    std::stringstream js;
    js << R"(
        const data = {
            type: 'surface',
            x: [)" << vectorToJSArray(strikes) << R"(],
            y: [)" << vectorToJSArray(maturities) << R"(],
            z: [)" << matrix2DToJSArray(surface) << R"(],
            colorscale: 'Viridis',
            contours: {
                z: {
                    show: true,
                    usecolormap: true,
                    highlightcolor: "#42f462",
                    project: {z: true}
                }
            }
        };
        
        const layout = {
            title: 'Volatility Surface',
            scene: {
                xaxis: {title: 'Strike Price'},
                yaxis: {title: 'Time to Maturity'},
                zaxis: {title: 'Implied Volatility'}
            },
            margin: {
                l: 65,
                r: 50,
                b: 65,
                t: 90,
            }
        };
        
        Plotly.newPlot('volSurfaceChart', [data], layout, {responsive: true});
    )";
    
    return js.str();
}

std::string ChartGenerator::generateOrderFlowImbalance(
    const std::vector<double>& buyVolume,
    const std::vector<double>& sellVolume) {
        
    std::stringstream js;
    js << R"(
        const buyData = {
            type: 'bar',
            x: Array.from({length: )" << buyVolume.size() << R"(}, (_, i) => i),
            y: [)" << vectorToJSArray(buyVolume) << R"(],
            name: 'Buy Volume',
            marker: {
                color: '#2ca02c'
            }
        };
        
        const sellData = {
            type: 'bar',
            x: Array.from({length: )" << sellVolume.size() << R"(}, (_, i) => i),
            y: [)" << vectorToJSArray(sellVolume) << R"(],
            name: 'Sell Volume',
            marker: {
                color: '#d62728'
            }
        };
        
        const layout = {
            title: 'Order Flow Imbalance',
            barmode: 'relative',
            xaxis: {
                title: 'Time',
                showgrid: true
            },
            yaxis: {
                title: 'Volume',
                showgrid: true
            }
        };
        
        Plotly.newPlot('orderFlowChart', [buyData, sellData], layout, {responsive: true});
    )";
    
    return js.str();
}

std::string ChartGenerator::generateCorrelationMatrix(
    const std::vector<std::vector<double>>& correlations,
    const std::vector<std::string>& labels) {
        
    std::stringstream js;
    js << R"(
        const data = {
            type: 'heatmap',
            z: [)" << matrix2DToJSArray(correlations) << R"(],
            x: [)" << vectorToJSArray(labels) << R"(],
            y: [)" << vectorToJSArray(labels) << R"(],
            colorscale: 'RdBu',
            zmin: -1,
            zmax: 1,
            showscale: true
        };
        
        const layout = {
            title: 'Correlation Matrix',
            xaxis: {
                ticks: '',
                side: 'bottom'
            },
            yaxis: {
                ticks: '',
                ticksuffix: ' ',
                autosize: true
            },
            margin: {
                l: 100,
                r: 50,
                b: 100,
                t: 50,
                pad: 4
            }
        };
        
        Plotly.newPlot('correlationChart', [data], layout, {responsive: true});
    )";
    
    return js.str();
}

// 辅助方法
template<typename T>
std::string ChartGenerator::vectorToJSArray(const std::vector<T>& vec) {
    std::stringstream ss;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) ss << ",";
        ss << vec[i];
    }
    return ss.str();
}

std::string ChartGenerator::matrix2DToJSArray(
    const std::vector<std::vector<double>>& matrix) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < matrix.size(); ++i) {
        if (i > 0) ss << ",";
        ss << "[" << vectorToJSArray(matrix[i]) << "]";
    }
    ss << "]";
    return ss.str();
}

} // namespace visualization
} // namespace hft
