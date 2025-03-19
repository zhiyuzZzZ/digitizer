# Git 仓库设置说明

此文件提供了将数字化模拟项目上传到Git仓库的指南。

## 已创建的文件

为了使项目适合Git管理，我们已经添加了以下文件：

- `.gitignore`: 排除构建目录、编译产物和临时文件
- `.gitattributes`: 确保在不同操作系统之间正确处理文件换行符
- `LICENSE`: MIT许可证文件
- `CONTRIBUTING.md`: 项目贡献指南
- `init_git.sh`: Git初始化脚本
- `scripts/plot_results.C`: 示例绘图脚本（如果不存在）

## 使用初始化脚本

我们提供了一个脚本来快速初始化Git仓库：

```bash
# 1. 进入项目目录
cd /path/to/digitizer

# 2. 添加脚本执行权限
chmod +x init_git.sh

# 3. 运行初始化脚本
./init_git.sh
```

该脚本将：
- 初始化一个新的Git仓库
- 根据`.gitignore`添加所有需要的文件
- 给出后续步骤的指导，如设置远程仓库等

## 手动初始化

如果您想手动控制过程，可以执行以下命令：

```bash
# 初始化Git仓库
git init

# 添加文件到暂存区
git add .

# 提交
git commit -m "初始提交 - 数字化模拟项目"

# 添加远程仓库（请替换为您的仓库URL）
git remote add origin https://github.com/username/repository.git

# 推送到远程仓库
git push -u origin master
```

## 注意事项

1. `build/`和`install/`目录被排除在版本控制之外，用户需要自行编译项目
2. 脚本中的环境变量设置（`setup.sh`）可能需要根据不同环境进行调整
3. 请确保项目中的任何敏感信息或大型数据文件已经添加到`.gitignore`文件中

## 代码维护

建议定期更新仓库，并使用有意义的提交消息：

```bash
# 添加所有更改
git add .

# 提交更改
git commit -m "描述性的提交消息"

# 推送到远程仓库
git push
```

如果您有任何问题，请参考`CONTRIBUTING.md`或创建GitHub issue。 