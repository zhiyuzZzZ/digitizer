#!/bin/bash

# 数字化模拟项目环境设置脚本

# 获取脚本所在目录（项目根目录）
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

echo "设置数字化模拟项目环境..."

# 检查ROOT环境
if [ -z "$ROOTSYS" ]; then
    echo "ROOT环境未设置，尝试加载..."
    
    # 尝试一些常见的ROOT路径
    if [ -f "/cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.26.04/x86_64-centos7-gcc48-opt/bin/thisroot.sh" ]; then
        source /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.26.04/x86_64-centos7-gcc48-opt/bin/thisroot.sh
    elif [ -f "/cvmfs/sft.cern.ch/lcg/views/LCG_105/x86_64-el9-gcc13-opt/setup.sh" ]; then
        source /cvmfs/sft.cern.ch/lcg/views/LCG_105/x86_64-el9-gcc13-opt/setup.sh
    else
        echo "警告: 无法自动设置ROOT环境，请手动设置。"
        echo "例如: source /path/to/root/bin/thisroot.sh"
    fi
else
    echo "ROOT环境已设置: $ROOTSYS"
fi

# 检查项目是否已编译
if [ -d "$SCRIPT_DIR/build" ]; then
    echo "检测到build目录，将其添加到PATH..."
    export PATH="$SCRIPT_DIR/build/bin:$PATH"
    echo "可执行文件路径: $SCRIPT_DIR/build/bin/digitize"
else
    echo "警告: 未检测到build目录，请先编译项目:"
    echo "  mkdir build && cd build && cmake .. && make"
fi

# 检查是否已安装到自定义目录
if [ -d "$HOME/.local/bin" ] && [ -f "$HOME/.local/bin/digitize" ]; then
    echo "检测到安装在用户目录，将其添加到PATH..."
    export PATH="$HOME/.local/bin:$PATH"
    echo "可执行文件路径: $HOME/.local/bin/digitize"
fi

# 创建必要的输出目录
mkdir -p "$SCRIPT_DIR/results/plots"
echo "已创建输出目录: $SCRIPT_DIR/results"

# 设置DIGITIZER_HOME环境变量
export DIGITIZER_HOME="$SCRIPT_DIR"
echo "设置DIGITIZER_HOME=$DIGITIZER_HOME"

# 创建别名简化命令
alias digi='digitize'
alias plot_results='root -l scripts/plot_results.C'
alias compare_digitizers='root -l scripts/compare_digitizers.C'
alias plot_scan='root -l scripts/plot_scan_results.C'

echo "已设置命令别名: digi, plot_results, compare_digitizers, plot_scan"

# 打印使用提示
echo ""
echo "环境设置完成! 现在可以运行以下命令:"
echo "  digitize --help     # 显示帮助信息"
echo "  digitize --all      # 运行所有数字化器"
echo "  digi --all --output results/test  # 使用别名并指定输出"
echo "  plot_results(\"results/test_Total.root\", \"results/plots/test\")  # 在ROOT中绘图"
echo ""
echo "提示: 使用 'mkdir -p results/自定义目录' 创建其他输出目录"
echo "" 