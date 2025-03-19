#!/bin/bash

# 数字化模拟项目的Git初始化脚本

echo "正在初始化Git仓库..."

# 初始化Git仓库
git init

# 添加远程仓库（用户需要修改为自己的仓库URL）
# git remote add origin https://github.com/username/repository.git

# 添加所有文件到暂存区，但会根据.gitignore排除不需要的文件
git add .

echo "Git仓库初始化完成！"
echo ""
echo "后续步骤:"
echo "1. 请设置您的Git用户名和邮箱（如果尚未设置）:"
echo "   git config user.name \"您的姓名\""
echo "   git config user.email \"您的邮箱\""
echo ""
echo "2. 提交初始代码:"
echo "   git commit -m \"初始提交 - 数字化模拟项目\""
echo ""
echo "3. 添加远程仓库（请替换为您的仓库URL）:"
echo "   git remote add origin https://github.com/username/repository.git"
echo ""
echo "4. 推送到远程仓库:"
echo "   git push -u origin master"
echo ""
echo "注意: 已自动创建.gitignore文件，编译目录和临时文件将不会被提交。" 