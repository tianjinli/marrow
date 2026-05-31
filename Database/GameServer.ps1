# 设置当前目录
$Host.UI.RawUI.WindowTitle = "网狐棋牌荣耀版一键安装"
$WorkDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$sqlcmdDir = Join-Path $WorkDir "mssql-tools18"
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
Write-Host "          执行网狐棋牌荣耀版数据库一键安装脚本，自动创建初始数据库。          "
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
$SqlArgs = @(
    '-S', $DB_HOST,
    '-U', $DB_USER,
    '-P', $DB_PASS,
    '-C',
    '-i'
)
# 打印时把-P 后面的密码替换为****
$SqlArgsClone = $SqlArgs.Clone()
# $SqlArgsClone[5] = "******"
Write-Host "当前命令行参数：$SqlArgsClone"

$WorkDir="$WorkDir\GameServer"

Write-Host ""
Write-Host "建立数据库"
$RootPath = "$WorkDir\1_1创建数据库"
& sqlcmd $SqlArgs "$RootPath\1_1_用户库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\1_2_平台库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\1_3_金币库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\1_4_记录库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\1_5_积分库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\1_6_比赛库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\1_7_练习库脚本.sql"

& sqlcmd $SqlArgs "$RootPath\2_1_用户库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\2_2_平台库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\2_3_金币库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\2_4_记录库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\2_5_积分库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\2_6_比赛库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\2_7_练习库脚本.sql"

Write-Host ""
Write-Host "建立链接服务器"
$RootPath="$WorkDir\1_2创建链接服务器"
& sqlcmd $SqlArgs "$RootPath\1_1用户链接.sql"
& sqlcmd $SqlArgs "$RootPath\1_2平台链接.sql"
& sqlcmd $SqlArgs "$RootPath\1_3金币链接.sql"
& sqlcmd $SqlArgs "$RootPath\1_4记录链接.sql"
& sqlcmd $SqlArgs "$RootPath\1_5积分链接.sql"
& sqlcmd $SqlArgs "$RootPath\1_6比赛链接.sql"
& sqlcmd $SqlArgs "$RootPath\1_7练习链接.sql"

Write-Host ""
Write-Host "建立初始数据"
$RootPath="$WorkDir\1_3创建初始数据\1_1用户初始数据"
& sqlcmd $SqlArgs "$RootPath\系统配置.sql"
& sqlcmd $SqlArgs "$RootPath\会员配置.sql"

$RootPath="$WorkDir\1_3创建初始数据\1_2平台初始数据"
& sqlcmd $SqlArgs "$RootPath\道具关系.sql"
& sqlcmd $SqlArgs "$RootPath\道具类型.sql"
& sqlcmd $SqlArgs "$RootPath\道具配置.sql"
& sqlcmd $SqlArgs "$RootPath\等级配置.sql"
& sqlcmd $SqlArgs "$RootPath\经验配置.sql"
& sqlcmd $SqlArgs "$RootPath\类型配置.sql"
& sqlcmd $SqlArgs "$RootPath\签到配置.sql"
& sqlcmd $SqlArgs "$RootPath\任务配置.sql"
& sqlcmd $SqlArgs "$RootPath\子道具配置.sql"

$RootPath="$WorkDir\1_3创建初始数据\1_3金币初始数据"
& sqlcmd $SqlArgs "$RootPath\返利配置.sql"
& sqlcmd $SqlArgs "$RootPath\列表配置.sql"

$RootPath="$WorkDir\1_3创建初始数据\1_5积分初始数据"
& sqlcmd $SqlArgs "$RootPath\列表配置.sql"

$RootPath="$WorkDir\1_3创建初始数据\1_6比赛初始数据"
& sqlcmd $SqlArgs "$RootPath\列表配置.sql"

$RootPath="$WorkDir\1_3创建初始数据\1_7练习初始数据"
& sqlcmd $SqlArgs "$RootPath\列表配置.sql"

Write-Host ""
Write-Host "建立存储过程"
$RootPath="$WorkDir\1_4创建存储过程\1_1用户数据库"
& sqlcmd $SqlArgs "$RootPath\绑定机器.sql"
& sqlcmd $SqlArgs "$RootPath\标识登录.sql"
& sqlcmd $SqlArgs "$RootPath\代理列表.sql"
& sqlcmd $SqlArgs "$RootPath\好友操作.sql"
& sqlcmd $SqlArgs "$RootPath\好友查找.sql"
& sqlcmd $SqlArgs "$RootPath\好友登录.sql"
& sqlcmd $SqlArgs "$RootPath\好友消息.sql"
& sqlcmd $SqlArgs "$RootPath\机器管理.sql"
& sqlcmd $SqlArgs "$RootPath\加载机器.sql"
& sqlcmd $SqlArgs "$RootPath\设置权限.sql"
& sqlcmd $SqlArgs "$RootPath\实名验证.sql"
& sqlcmd $SqlArgs "$RootPath\校验权限.sql"
& sqlcmd $SqlArgs "$RootPath\校验资料.sql"
& sqlcmd $SqlArgs "$RootPath\修改密码.sql"
& sqlcmd $SqlArgs "$RootPath\修改签名.sql"
& sqlcmd $SqlArgs "$RootPath\用户资料.sql"
& sqlcmd $SqlArgs "$RootPath\帐号绑定.sql"
& sqlcmd $SqlArgs "$RootPath\帐号登录.sql"
& sqlcmd $SqlArgs "$RootPath\注册帐号.sql"
& sqlcmd $SqlArgs "$RootPath\自定头像.sql"

$RootPath="$WorkDir\1_4创建存储过程\1_2平台数据库"
& sqlcmd $SqlArgs "$RootPath\背包管理.sql"
& sqlcmd $SqlArgs "$RootPath\道具管理.sql"
& sqlcmd $SqlArgs "$RootPath\等级管理.sql"
& sqlcmd $SqlArgs "$RootPath\低保管理.sql"
& sqlcmd $SqlArgs "$RootPath\房间管理.sql"
& sqlcmd $SqlArgs "$RootPath\会员管理.sql"
& sqlcmd $SqlArgs "$RootPath\加载节点.sql"
& sqlcmd $SqlArgs "$RootPath\加载类型.sql"
& sqlcmd $SqlArgs "$RootPath\加载敏感词.sql"
& sqlcmd $SqlArgs "$RootPath\加载页面.sql"
& sqlcmd $SqlArgs "$RootPath\加载种类.sql"
& sqlcmd $SqlArgs "$RootPath\喇叭使用.sql"
& sqlcmd $SqlArgs "$RootPath\连接信息.sql"
& sqlcmd $SqlArgs "$RootPath\模块管理.sql"
& sqlcmd $SqlArgs "$RootPath\平台配置.sql"
& sqlcmd $SqlArgs "$RootPath\签到管理.sql"
& sqlcmd $SqlArgs "$RootPath\任务管理.sql"
& sqlcmd $SqlArgs "$RootPath\实名配置.sql"
& sqlcmd $SqlArgs "$RootPath\在线信息.sql"

$RootPath="$WorkDir\1_4创建存储过程\1_3金币数据库"
& sqlcmd $SqlArgs "$RootPath\标识登录.sql"
& sqlcmd $SqlArgs "$RootPath\查询用户.sql"
& sqlcmd $SqlArgs "$RootPath\兑换管理.sql"
& sqlcmd $SqlArgs "$RootPath\加载机器.sql"
& sqlcmd $SqlArgs "$RootPath\机器配置.sql"
& sqlcmd $SqlArgs "$RootPath\加载配置.sql"
& sqlcmd $SqlArgs "$RootPath\加载消息.sql"
& sqlcmd $SqlArgs "$RootPath\离开房间.sql"
& sqlcmd $SqlArgs "$RootPath\列表描述.sql"
& sqlcmd $SqlArgs "$RootPath\设置权限.sql"
& sqlcmd $SqlArgs "$RootPath\推广管理.sql"
& sqlcmd $SqlArgs "$RootPath\写入费用.sql"
& sqlcmd $SqlArgs "$RootPath\银行服务.sql"
& sqlcmd $SqlArgs "$RootPath\游戏记录.sql"
& sqlcmd $SqlArgs "$RootPath\游戏数据.sql"
& sqlcmd $SqlArgs "$RootPath\游戏写分.sql"

$RootPath="$WorkDir\1_4创建存储过程\1_5积分数据库"
& sqlcmd $SqlArgs "$RootPath\标识登录.sql"
& sqlcmd $SqlArgs "$RootPath\加载配置.sql"
& sqlcmd $SqlArgs "$RootPath\加载消息.sql"
& sqlcmd $SqlArgs "$RootPath\离开房间.sql"
& sqlcmd $SqlArgs "$RootPath\列表描述.sql"
& sqlcmd $SqlArgs "$RootPath\设置权限.sql"
& sqlcmd $SqlArgs "$RootPath\游戏记录.sql"
& sqlcmd $SqlArgs "$RootPath\游戏写分.sql"

$RootPath="$WorkDir\1_4创建存储过程\1_6比赛数据库"
& sqlcmd $SqlArgs "$RootPath\比赛管理.sql"
& sqlcmd $SqlArgs "$RootPath\标识登录.sql"
& sqlcmd $SqlArgs "$RootPath\加载配置.sql"
& sqlcmd $SqlArgs "$RootPath\加载消息.sql"
& sqlcmd $SqlArgs "$RootPath\开始结束.sql"
& sqlcmd $SqlArgs "$RootPath\离开房间.sql"
& sqlcmd $SqlArgs "$RootPath\列表描述.sql"
& sqlcmd $SqlArgs "$RootPath\设置权限.sql"
& sqlcmd $SqlArgs "$RootPath\写入费用.sql"
& sqlcmd $SqlArgs "$RootPath\写入奖励.sql"
& sqlcmd $SqlArgs "$RootPath\游戏记录.sql"
& sqlcmd $SqlArgs "$RootPath\游戏写分.sql"

$RootPath="$WorkDir\1_4创建存储过程\1_7练习数据库"
& sqlcmd $SqlArgs "$RootPath\标识登录.sql"
& sqlcmd $SqlArgs "$RootPath\加载配置.sql"
& sqlcmd $SqlArgs "$RootPath\加载消息.sql"
& sqlcmd $SqlArgs "$RootPath\离开房间.sql"
& sqlcmd $SqlArgs "$RootPath\列表描述.sql"
& sqlcmd $SqlArgs "$RootPath\设置权限.sql"
& sqlcmd $SqlArgs "$RootPath\游戏记录.sql"
& sqlcmd $SqlArgs "$RootPath\游戏写分.sql"

$RootPath="$WorkDir\1_6私人房间"
& sqlcmd $SqlArgs "$RootPath\房间费用.sql"
& sqlcmd $SqlArgs "$RootPath\房间管理.sql"
& sqlcmd $SqlArgs "$RootPath\房间配置.sql"
& sqlcmd $SqlArgs "$RootPath\房卡管理.sql"
& sqlcmd $SqlArgs "$RootPath\房卡信息.sql"
& sqlcmd $SqlArgs "$RootPath\参与信息.sql"
& sqlcmd $SqlArgs "$RootPath\积分写分.sql"
& sqlcmd $SqlArgs "$RootPath\金币写分.sql"

Write-Host ""
Write-Host "*************************************************************************"
Write-Host "              ！！！网狐棋牌荣耀版一键安装脚本已经执行完毕！！！             "
Write-Host "                        无需生成游戏标识，请直接关闭                       "
Write-Host "*************************************************************************"
Pause

Clear-Host
Write-Host ""
Write-Host "建立游戏标识"
$RootPath = "$WorkDir\1_5创建游戏标识\"
& sqlcmd $SqlArgs "$RootPath`标识生成.sql"

Clear-Host
Write-Host "*************************************************************************"
Write-Host "                                                                         "
Write-Host "                        ！！！创建游戏标识完毕！！！                        "
Write-Host "                                                                         "
Write-Host "                     版权所有： 深圳市网狐科技有限公司                      "
Write-Host "*************************************************************************"
Pause
