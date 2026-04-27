# 设置当前目录
$Host.UI.RawUI.WindowTitle = "网狐棋牌精华版一键安装"
$workDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$sqlcmdDir = Join-Path $workDir "mssql-tools18"
$env:PATH += ";$sqlcmdDir;$PSScriptRoot"

# 检查是否已存在
if ((Get-Command sqlcmd -ErrorAction SilentlyContinue) -or (Test-Path (Join-Path $sqlcmdDir "sqlcmd.exe"))) {
    Write-Host "✅ 已存在 sqlcmd，跳过下载。"
} else {
    $downloadPath   = Join-Path $sqlcmdDir "sqlcmd-windows-amd64.zip"
    $downloadUrl    = "https://github.com/microsoft/go-sqlcmd/releases/download/v1.8.2/sqlcmd-windows-amd64.zip"
    # 目录不存在则创建
    if (-not (Test-Path $sqlcmdDir)) {
        New-Item -ItemType Directory -Path $sqlcmdDir | Out-Null
    }

    Write-Host "⬇️ 正在下载 $downloadUrl ..."
    Invoke-WebRequest -Uri $downloadUrl -OutFile $downloadPath

    Write-Host "📦 正在解压到 $sqlcmdDir ..."
    if (-not (Test-Path $sqlcmdDir)) {
        New-Item -ItemType Directory -Path $sqlcmdDir | Out-Null
    }
    Expand-Archive -Path $downloadPath -DestinationPath $sqlcmdDir -Force

    Write-Host "✅ 安装完成，sqlcmd 位于 $sqlcmdDir"
}

Write-Host "*************************************************************************"
Write-Host "          执行网狐棋牌精华版数据库一键安装脚本，自动创建初始数据库。          "
Write-Host "*************************************************************************"
Pause
Write-Host ""

# https://github.com/microsoft/go-sqlcmd
# SQL 参数 (支持 osql 和 sqlcmd)
# -S 192.168.159.138 -U sa -P poTP9YjMrLhVhm5B -C -i
$DB_HOST = $env:DB_HOST
$DB_USER = $env:DB_USER
$DB_PASS = $env:DB_PASS
if ([string]::IsNullOrEmpty($DB_HOST)) {
    $DB_HOST = "localhost"
}
if ([string]::IsNullOrEmpty($DB_USER)) {
    $DB_USER = "sa"
}
if ([string]::IsNullOrEmpty($DB_PASS)) {
    $DB_PASS = "poTP9YjMrLhVhm5B"
}
$sqlArgs = @(
    '-S', $DB_HOST,
    '-U', $DB_USER,
    '-P', $DB_PASS,
    '-C',
    '-i'
)
# 打印时把-P 后面的密码替换为****
$sqlArgsClone = $sqlArgs.Clone()
# $sqlArgsClone[5] = "******"
Write-Host "当前命令行参数：$sqlArgsClone"

Write-Host ""
Write-Host "删除数据库"
$rootPath = "$workDir\1_1创建数据库\"
& sqlcmd $sqlArgs "$rootPath`数据库删除.sql"

Write-Host ""
Write-Host "建立数据库"
$rootPath = "$workDir\1_1创建数据库\"
& sqlcmd $sqlArgs "$rootPath`1_1_用户库脚本.sql"
& sqlcmd $sqlArgs "$rootPath`1_2_平台库脚本.sql"
& sqlcmd $sqlArgs "$rootPath`1_3_金币库脚本.sql"
& sqlcmd $sqlArgs "$rootPath`1_4_记录库脚本.sql"
& sqlcmd $sqlArgs "$rootPath`1_5_积分库脚本.sql"
& sqlcmd $sqlArgs "$rootPath`1_6_比赛库脚本.sql"
& sqlcmd $sqlArgs "$rootPath`1_7_练习库脚本.sql"

& sqlcmd $sqlArgs "$rootPath`2_1_用户库脚本.sql"
& sqlcmd $sqlArgs "$rootPath`2_2_平台库脚本.sql"
& sqlcmd $sqlArgs "$rootPath`2_3_金币库脚本.sql"
& sqlcmd $sqlArgs "$rootPath`2_4_记录库脚本.sql"
& sqlcmd $sqlArgs "$rootPath`2_5_积分库脚本.sql"
& sqlcmd $sqlArgs "$rootPath`2_6_比赛库脚本.sql"
& sqlcmd $sqlArgs "$rootPath`2_7_练习库脚本.sql"

Write-Host ""
Write-Host "建立链接服务器"
$rootPath = "$workDir\1_2创建链接服务器\"
& sqlcmd $sqlArgs "$rootPath`1_1用户链接.sql"
& sqlcmd $sqlArgs "$rootPath`1_2平台链接.sql"
& sqlcmd $sqlArgs "$rootPath`1_3金币链接.sql"
& sqlcmd $sqlArgs "$rootPath`1_4记录链接.sql"
& sqlcmd $sqlArgs "$rootPath`1_5积分链接.sql"
& sqlcmd $sqlArgs "$rootPath`1_6比赛链接.sql"
& sqlcmd $sqlArgs "$rootPath`1_7练习链接.sql"

Write-Host ""
Write-Host "建立初始数据"
$rootPath = "$workDir\1_3创建初始数据\1_1用户初始数据\"
& sqlcmd $sqlArgs "$rootPath`系统配置.sql"
& sqlcmd $sqlArgs "$rootPath`会员配置.sql"

$rootPath = "$workDir\1_3创建初始数据\1_2平台初始数据\"
& sqlcmd $sqlArgs "$rootPath`道具关系.sql"
& sqlcmd $sqlArgs "$rootPath`道具类型.sql"
& sqlcmd $sqlArgs "$rootPath`道具配置.sql"
& sqlcmd $sqlArgs "$rootPath`类型配置.sql"
& sqlcmd $sqlArgs "$rootPath`签到配置.sql"
& sqlcmd $sqlArgs "$rootPath`子道具配置.sql"

$rootPath = "$workDir\1_3创建初始数据\1_3金币初始数据\"
& sqlcmd $sqlArgs "$rootPath`返利配置.sql"
& sqlcmd $sqlArgs "$rootPath`列表配置.sql"

$rootPath = "$workDir\1_3创建初始数据\1_5积分初始数据\"
& sqlcmd $sqlArgs "$rootPath`列表配置.sql"

$rootPath = "$workDir\1_3创建初始数据\1_6比赛初始数据\"
& sqlcmd $sqlArgs "$rootPath`列表配置.sql"

$rootPath = "$workDir\1_3创建初始数据\1_7练习初始数据\"
& sqlcmd $sqlArgs "$rootPath`列表配置.sql"

Write-Host ""
Write-Host "建立存储过程"
$rootPath = "$workDir\1_4创建存储过程\1_1用户数据库\"
& sqlcmd $sqlArgs "$rootPath`绑定机器.sql"
& sqlcmd $sqlArgs "$rootPath`标识登录.sql"
& sqlcmd $sqlArgs "$rootPath`代理列表.sql"
& sqlcmd $sqlArgs "$rootPath`好友操作.sql"
& sqlcmd $sqlArgs "$rootPath`好友查找.sql"
& sqlcmd $sqlArgs "$rootPath`好友登录.sql"
& sqlcmd $sqlArgs "$rootPath`好友消息.sql"
& sqlcmd $sqlArgs "$rootPath`机器管理.sql"
& sqlcmd $sqlArgs "$rootPath`加载机器.sql"
& sqlcmd $sqlArgs "$rootPath`设置权限.sql"
& sqlcmd $sqlArgs "$rootPath`实名验证.sql"
& sqlcmd $sqlArgs "$rootPath`校验权限.sql"
& sqlcmd $sqlArgs "$rootPath`校验资料.sql"
& sqlcmd $sqlArgs "$rootPath`修改密码.sql"
& sqlcmd $sqlArgs "$rootPath`修改签名.sql"
& sqlcmd $sqlArgs "$rootPath`用户资料.sql"
& sqlcmd $sqlArgs "$rootPath`帐号绑定.sql"
& sqlcmd $sqlArgs "$rootPath`帐号登录.sql"
& sqlcmd $sqlArgs "$rootPath`注册帐号.sql"
& sqlcmd $sqlArgs "$rootPath`自定头像.sql"

$rootPath = "$workDir\1_4创建存储过程\1_2平台数据库\"
& sqlcmd $sqlArgs "$rootPath`背包管理.sql"
& sqlcmd $sqlArgs "$rootPath`道具管理.sql"
& sqlcmd $sqlArgs "$rootPath`房间管理.sql"
& sqlcmd $sqlArgs "$rootPath`会员管理.sql"
& sqlcmd $sqlArgs "$rootPath`加载节点.sql"
& sqlcmd $sqlArgs "$rootPath`加载类型.sql"
& sqlcmd $sqlArgs "$rootPath`加载敏感词.sql"
& sqlcmd $sqlArgs "$rootPath`加载页面.sql"
& sqlcmd $sqlArgs "$rootPath`加载种类.sql"
& sqlcmd $sqlArgs "$rootPath`喇叭使用.sql"
& sqlcmd $sqlArgs "$rootPath`连接信息.sql"
& sqlcmd $sqlArgs "$rootPath`加载消息.sql"
& sqlcmd $sqlArgs "$rootPath`模块管理.sql"
& sqlcmd $sqlArgs "$rootPath`平台配置.sql"
& sqlcmd $sqlArgs "$rootPath`签到管理.sql"
& sqlcmd $sqlArgs "$rootPath`任务管理.sql"
& sqlcmd $sqlArgs "$rootPath`实名配置.sql"
& sqlcmd $sqlArgs "$rootPath`视频管理.sql"
& sqlcmd $sqlArgs "$rootPath`在线信息.sql"

$rootPath = "$workDir\1_4创建存储过程\1_3金币数据库\"
& sqlcmd $sqlArgs "$rootPath`标识登录.sql"
& sqlcmd $sqlArgs "$rootPath`查询用户.sql"
& sqlcmd $sqlArgs "$rootPath`兑换管理.sql"
& sqlcmd $sqlArgs "$rootPath`加载机器.sql"
& sqlcmd $sqlArgs "$rootPath`机器配置.sql"
& sqlcmd $sqlArgs "$rootPath`加载配置.sql"
& sqlcmd $sqlArgs "$rootPath`俱乐部管理.sql"
& sqlcmd $sqlArgs "$rootPath`离开房间.sql"
& sqlcmd $sqlArgs "$rootPath`列表描述.sql"
& sqlcmd $sqlArgs "$rootPath`设置权限.sql"
& sqlcmd $sqlArgs "$rootPath`视频服务.sql"
& sqlcmd $sqlArgs "$rootPath`推广管理.sql"
& sqlcmd $sqlArgs "$rootPath`写入费用.sql"
& sqlcmd $sqlArgs "$rootPath`银行服务.sql"
& sqlcmd $sqlArgs "$rootPath`游戏记录.sql"
& sqlcmd $sqlArgs "$rootPath`游戏数据.sql"
& sqlcmd $sqlArgs "$rootPath`游戏写分.sql"

$rootPath = "$workDir\1_4创建存储过程\1_5积分数据库\"
& sqlcmd $sqlArgs "$rootPath`标识登录.sql"
& sqlcmd $sqlArgs "$rootPath`加载配置.sql"
& sqlcmd $sqlArgs "$rootPath`离开房间.sql"
& sqlcmd $sqlArgs "$rootPath`列表描述.sql"
& sqlcmd $sqlArgs "$rootPath`设置权限.sql"
& sqlcmd $sqlArgs "$rootPath`游戏记录.sql"
& sqlcmd $sqlArgs "$rootPath`游戏写分.sql"

$rootPath = "$workDir\1_4创建存储过程\1_6比赛数据库\"
& sqlcmd $sqlArgs "$rootPath`比赛管理.sql"
& sqlcmd $sqlArgs "$rootPath`标识登录.sql"
& sqlcmd $sqlArgs "$rootPath`加载配置.sql"
& sqlcmd $sqlArgs "$rootPath`加载消息.sql"
& sqlcmd $sqlArgs "$rootPath`开始结束.sql"
& sqlcmd $sqlArgs "$rootPath`离开房间.sql"
& sqlcmd $sqlArgs "$rootPath`列表描述.sql"
& sqlcmd $sqlArgs "$rootPath`设置权限.sql"
& sqlcmd $sqlArgs "$rootPath`写入费用.sql"
& sqlcmd $sqlArgs "$rootPath`写入奖励.sql"
& sqlcmd $sqlArgs "$rootPath`游戏记录.sql"
& sqlcmd $sqlArgs "$rootPath`游戏写分.sql"

$rootPath = "$workDir\1_4创建存储过程\1_7练习数据库\"
& sqlcmd $sqlArgs "$rootPath`标识登录.sql"
& sqlcmd $sqlArgs "$rootPath`加载配置.sql"
& sqlcmd $sqlArgs "$rootPath`加载消息.sql"
& sqlcmd $sqlArgs "$rootPath`离开房间.sql"
& sqlcmd $sqlArgs "$rootPath`列表描述.sql"
& sqlcmd $sqlArgs "$rootPath`设置权限.sql"
& sqlcmd $sqlArgs "$rootPath`游戏记录.sql"
& sqlcmd $sqlArgs "$rootPath`游戏写分.sql"

$rootPath = "$workDir\1_6私人房间\"
& sqlcmd $sqlArgs "$rootPath`房间费用.sql"
& sqlcmd $sqlArgs "$rootPath`底分配置.sql"
& sqlcmd $sqlArgs "$rootPath`房间管理.sql"
& sqlcmd $sqlArgs "$rootPath`房间配置.sql"
& sqlcmd $sqlArgs "$rootPath`房卡管理.sql"
& sqlcmd $sqlArgs "$rootPath`房卡信息.sql"
& sqlcmd $sqlArgs "$rootPath`参与信息.sql"
& sqlcmd $sqlArgs "$rootPath`积分写分.sql"
& sqlcmd $sqlArgs "$rootPath`回放信息.sql"
& sqlcmd $sqlArgs "$rootPath`金币写分.sql"

Write-Host ""
Write-Host "*************************************************************************"
Write-Host "              ！！！网狐棋牌精华版一键安装脚本已经执行完毕！！！             "
Write-Host "                        无需生成游戏标识，请直接关闭                       "
Write-Host "*************************************************************************"
Pause

Clear-Host
Write-Host ""
Write-Host "建立游戏标识"
$rootPath = "$workDir\1_5创建游戏标识\"
& sqlcmd $sqlArgs "$rootPath`标识生成.sql"

Clear-Host
Write-Host "*************************************************************************"
Write-Host "                                                                         "
Write-Host "                        ！！！创建游戏标识完毕！！！                        "
Write-Host "                                                                         "
Write-Host "                     版权所有： 深圳市网狐科技有限公司                      "
Write-Host "*************************************************************************"
Pause
