# 网狐荣耀客户端

- [Cocos 资料大全](https://github.com/fusijie/Cocos-Resource)

## 下载解压引擎

1. 下载客户端引擎
```cmd
curl http://digitalocean.cocos2d-x.org/Cocos2D-X/cocos2d-x-3.10.zip -o cocos2d-x-3.10.zip
```
```cmd
powershell -Command "(New-Object Net.WebClient).DownloadFile('http://digitalocean.cocos2d-x.org/Cocos2D-X/cocos2d-x-3.10.zip', 'cocos2d-x-3.10.zip')"
```
2. 解压客户端引擎
请注意目录结构: CHANGELOG 文件相对路径为 `GameClient\frameworks\cocos2d-x\CHANGELOG`
```cmd
powershell -command ^
"Add-Type -AssemblyName 'System.IO.Compression.FileSystem';" ^
";[System.IO.Compression.ZipFile]::ExtractToDirectory('cocos2d-x-3.10.zip', 'frameworks\cocos2d-x')"
```

## 编译调试程序

### 全局配置
1. 设置输出目录
$(SolutionDir)..\..\..\..\_Out\bin\$(Configuration)\RYClient
2. 设置中间目录
$(SolutionDir)..\..\..\..\_Out\obj\$(Configuration)\RYClient
3. 设置平台工具集
$(DefaultPlatformToolset)
建议使用 v141 (2017) 平台工具集

### 主项目配置
1. 设置 `链接器 -> 常规 -> 输出文件`
恢复默认值 `$(OutDir)$(TargetName)$(TargetExt)`
2. 设置 `链接器 -> 调试 -> 生成程序数据库文件`
恢复默认值 `$(OutDir)$(TargetName).pdb`
3. 设置 `常规 -> 工作目录`
$(SolutionDir)..\..\..\..\_Out\bin\$(Configuration)\RYClient
4. 删除 `自定义生成步骤 -> 常规 -> 命令行`
if not exist "$(LocalDebuggerWorkingDirectory)" mkdir "$(LocalDebuggerWorkingDirectory)"
xcopy /Y /Q "$(OutDir)*.dll" "$(LocalDebuggerWorkingDirectory)"
xcopy /Y /Q "$(ProjectDir)..\Classes\ide-support\lang" "$(LocalDebuggerWorkingDirectory)"
xcopy "$(ProjectDir)..\..\..\client\base" "$(LocalDebuggerWorkingDirectory)\base" /D /E /I /F /Y

## 连接地址修改

1. 打开 `GameClient/client/client/src/plaza/models/yl.lua` 文件
2. 修改 `yl.SERVER_LIST` 列表中的地址为你的登录服务器地址
3. 修改 `yl.CURRENT_INDEX` 为对应服务器的索引（从 1 开始）
4. 打开 `GameClient/client/base/src/app/models/AppDF.lua` 文件
5. 修改 `appdf.ENV` 为对应环境（1.本地 2.测试 3.线上）
6. 修改 `appdf.HTTP_URLS` 列表中的地址为你的网站服务器地址
7. 修改 `appdf.SERVER_LIST` 列表中的地址为你的游戏服务器地址
