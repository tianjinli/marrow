# 设置当前目录
$Host.UI.RawUI.WindowTitle = "网狐棋牌荣耀版一键安装"
$WorkDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$SqlcmdDir = Join-Path $WorkDir "mssql-tools18"
$env:PATH += ";$SqlcmdDir;$PSScriptRoot"

# 检查是否已存在
if ((Get-Command sqlcmd -ErrorAction SilentlyContinue) -or (Test-Path (Join-Path $SqlcmdDir "sqlcmd.exe"))) {
    Write-Host "✅ 已存在 sqlcmd，跳过下载。"
} else {
    $downloadPath   = Join-Path $SqlcmdDir "sqlcmd-windows-amd64.zip"
    $downloadUrl    = "https://github.com/microsoft/go-sqlcmd/releases/download/v1.8.2/sqlcmd-windows-amd64.zip"
    # 目录不存在则创建
    if (-not (Test-Path $SqlcmdDir)) {
        New-Item -ItemType Directory -Path $SqlcmdDir | Out-Null
    }

    Write-Host "⬇️ 正在下载 $downloadUrl ..."
    Invoke-WebRequest -Uri $downloadUrl -OutFile $downloadPath

    Write-Host "📦 正在解压到 $SqlcmdDir ..."
    if (-not (Test-Path $SqlcmdDir)) {
        New-Item -ItemType Directory -Path $SqlcmdDir | Out-Null
    }
    Expand-Archive -Path $downloadPath -DestinationPath $SqlcmdDir -Force

    Write-Host "✅ 安装完成，sqlcmd 位于 $SqlcmdDir"
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
function Invoke-SqlCmdWithDb {
    param(
        [string]$Database,
        [string]$SqlFile
    )

    $SqlArgs = @(
        '-S', $DB_HOST,
        '-U', $DB_USER,
        '-P', $DB_PASS,
        '-d', $Database,
        '-C',
        '-i'
    )
    # 执行 sqlcmd 命令
    & sqlcmd $SqlArgs $SqlFile
}

# 打印时把-P 后面的密码替换为****
$SqlArgsClone = $SqlArgs.Clone()
# $SqlArgsClone[5] = "******"
Write-Host "当前命令行参数：$SqlArgsClone"

$WorkDir="$WorkDir\WebServer"

Write-Host ""
Write-Host "数据库脚本"
$RootPath="$WorkDir\1.数据库脚本"
& sqlcmd $SqlArgs "$RootPath\1.数据库删除.sql"
& sqlcmd $SqlArgs "$RootPath\1_1_网站库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\1_2_后台库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\2_1_网站库脚本.sql"
& sqlcmd $SqlArgs "$RootPath\2_2_后台库脚本.sql"

Write-Host ""
Write-Host "数据脚本"
$RootPath="$WorkDir\2.数据脚本"
& sqlcmd $SqlArgs "$RootPath\充值服务.sql"
& sqlcmd $SqlArgs "$RootPath\后台数据.sql"
& sqlcmd $SqlArgs "$RootPath\实卡类型.sql"
& sqlcmd $SqlArgs "$RootPath\推广数据.sql"
& sqlcmd $SqlArgs "$RootPath\泡点设置.sql"
& sqlcmd $SqlArgs "$RootPath\独立页面.sql"
& sqlcmd $SqlArgs "$RootPath\站点配置.sql"
& sqlcmd $SqlArgs "$RootPath\系统广告.sql"
& sqlcmd $SqlArgs "$RootPath\网站链接.sql"
& sqlcmd $SqlArgs "$RootPath\转盘数据.sql"
& sqlcmd $SqlArgs "$RootPath\会员属性.sql"

Write-Host ""
Write-Host "存储过程"
$RootPath="$WorkDir\3.存储过程\作业脚本"
& sqlcmd $SqlArgs "$RootPath\每日统计(作业).sql"
& sqlcmd $SqlArgs "$RootPath\统计玩家税收(作业).sql"
& sqlcmd $SqlArgs "$RootPath\统计代理充值(作业).sql"

$RootPath="$WorkDir\3.存储过程\公共过程"
Invoke-SqlCmdWithDb -Database "RYAccountsDB" -SqlFile "$RootPath\分页过程.sql"
Invoke-SqlCmdWithDb -Database "RYGameMatchDB" -SqlFile "$RootPath\分页过程.sql"
Invoke-SqlCmdWithDb -Database "RYGameScoreDB" -SqlFile "$RootPath\分页过程.sql"
Invoke-SqlCmdWithDb -Database "RYNativeWebDB" -SqlFile "$RootPath\分页过程.sql"
Invoke-SqlCmdWithDb -Database "RYPlatformDB" -SqlFile "$RootPath\分页过程.sql"
Invoke-SqlCmdWithDb -Database "RYPlatformManagerDB" -SqlFile "$RootPath\分页过程.sql"
Invoke-SqlCmdWithDb -Database "RYRecordDB" -SqlFile "$RootPath\分页过程.sql"
Invoke-SqlCmdWithDb -Database "RYTreasureDB" -SqlFile "$RootPath\分页过程.sql"

Invoke-SqlCmdWithDb -Database "RYAccountsDB" -SqlFile "$RootPath\切字符串.sql"
Invoke-SqlCmdWithDb -Database "RYGameMatchDB" -SqlFile "$RootPath\切字符串.sql"
Invoke-SqlCmdWithDb -Database "RYGameScoreDB" -SqlFile "$RootPath\切字符串.sql"
Invoke-SqlCmdWithDb -Database "RYNativeWebDB" -SqlFile "$RootPath\切字符串.sql"
Invoke-SqlCmdWithDb -Database "RYPlatformDB" -SqlFile "$RootPath\切字符串.sql"
Invoke-SqlCmdWithDb -Database "RYPlatformManagerDB" -SqlFile "$RootPath\切字符串.sql"
Invoke-SqlCmdWithDb -Database "RYRecordDB" -SqlFile "$RootPath\切字符串.sql"
Invoke-SqlCmdWithDb -Database "RYTreasureDB" -SqlFile "$RootPath\切字符串.sql"

$RootPath="$WorkDir\3.存储过程\函数"
& sqlcmd $SqlArgs "$RootPath\查询指定玩家的代理玩家.sql"

$RootPath="$WorkDir\3.存储过程\前台脚本\本地数据库"
& sqlcmd $SqlArgs "$RootPath\推荐游戏.sql"
& sqlcmd $SqlArgs "$RootPath\购买奖品.sql"

$RootPath="$WorkDir\3.存储过程\前台脚本\比赛数据库"
& sqlcmd $SqlArgs "$RootPath\比赛排行.sql"

$RootPath="$WorkDir\3.存储过程\前台脚本\用户数据库"
& sqlcmd $SqlArgs "$RootPath\修改密码.sql"
& sqlcmd $SqlArgs "$RootPath\修改资料.sql"
& sqlcmd $SqlArgs "$RootPath\固定机器.sql"
& sqlcmd $SqlArgs "$RootPath\奖牌兑换.sql"
& sqlcmd $SqlArgs "$RootPath\每日签到.sql"
& sqlcmd $SqlArgs "$RootPath\用户全局信息.sql"
& sqlcmd $SqlArgs "$RootPath\用户名检测.sql"
& sqlcmd $SqlArgs "$RootPath\用户注册.sql"
& sqlcmd $SqlArgs "$RootPath\用户登录.sql"
& sqlcmd $SqlArgs "$RootPath\获取用户信息.sql"
& sqlcmd $SqlArgs "$RootPath\账户保护.sql"
& sqlcmd $SqlArgs "$RootPath\重置密码.sql"
& sqlcmd $SqlArgs "$RootPath\魅力兑换.sql"
& sqlcmd $SqlArgs "$RootPath\自定头像.sql"

$RootPath="$WorkDir\3.存储过程\前台脚本\积分数据库"
& sqlcmd $SqlArgs "$RootPath\负分清零.sql"
& sqlcmd $SqlArgs "$RootPath\逃率清零.sql"

$RootPath="$WorkDir\3.存储过程\前台脚本\网站数据库"
& sqlcmd $SqlArgs "$RootPath\更新浏览.sql"
& sqlcmd $SqlArgs "$RootPath\比赛报名.sql"
& sqlcmd $SqlArgs "$RootPath\获取新闻.sql"
& sqlcmd $SqlArgs "$RootPath\购买奖品.sql"
& sqlcmd $SqlArgs "$RootPath\问题反馈.sql"

$RootPath="$WorkDir\3.存储过程\前台脚本\金币数据库"
& sqlcmd $SqlArgs "$RootPath\代理结算.sql"
& sqlcmd $SqlArgs "$RootPath\在线充值.sql"
& sqlcmd $SqlArgs "$RootPath\在线订单.sql"
& sqlcmd $SqlArgs "$RootPath\实卡充值.sql"
& sqlcmd $SqlArgs "$RootPath\推广中心.sql"
& sqlcmd $SqlArgs "$RootPath\推广信息.sql"
& sqlcmd $SqlArgs "$RootPath\苹果充值.sql"
& sqlcmd $SqlArgs "$RootPath\金币取款.sql"
& sqlcmd $SqlArgs "$RootPath\金币存款.sql"
& sqlcmd $SqlArgs "$RootPath\金币转账.sql"
& sqlcmd $SqlArgs "$RootPath\手游充值.sql"
& sqlcmd $SqlArgs "$RootPath\分享赠送.sql"
& sqlcmd $SqlArgs "$RootPath\转盘抽奖.sql"

$RootPath="$WorkDir\3.存储过程\后台脚本\帐号库"
& sqlcmd $SqlArgs "$RootPath\插入限制IP.sql"
& sqlcmd $SqlArgs "$RootPath\插入限制机器码.sql"
& sqlcmd $SqlArgs "$RootPath\更新用户.sql"
& sqlcmd $SqlArgs "$RootPath\注册IP统计.sql"
& sqlcmd $SqlArgs "$RootPath\注册机器码统计.sql"
& sqlcmd $SqlArgs "$RootPath\添加用户.sql"
& sqlcmd $SqlArgs "$RootPath\创建代理.sql"

$RootPath="$WorkDir\3.存储过程\后台脚本\平台库"
& sqlcmd $SqlArgs "$RootPath\在线统计.sql"

$RootPath="$WorkDir\3.存储过程\后台脚本\数据分析"
& sqlcmd $SqlArgs "$RootPath\充值统计.sql"
& sqlcmd $SqlArgs "$RootPath\其他统计.sql"
& sqlcmd $SqlArgs "$RootPath\活跃统计.sql"
& sqlcmd $SqlArgs "$RootPath\用户统计.sql"
& sqlcmd $SqlArgs "$RootPath\金币分布.sql"

$RootPath="$WorkDir\3.存储过程\后台脚本\权限库"
& sqlcmd $SqlArgs "$RootPath\权限加载.sql"
& sqlcmd $SqlArgs "$RootPath\用户表操作.sql"
& sqlcmd $SqlArgs "$RootPath\管理员登录.sql"
& sqlcmd $SqlArgs "$RootPath\菜单加载.sql"

$RootPath="$WorkDir\3.存储过程\后台脚本\比赛库"
& sqlcmd $SqlArgs "$RootPath\比赛排名.sql"

$RootPath="$WorkDir\3.存储过程\后台脚本\积分库"
& sqlcmd $SqlArgs "$RootPath\清零积分.sql"
& sqlcmd $SqlArgs "$RootPath\清零逃率.sql"
& sqlcmd $SqlArgs "$RootPath\赠送积分.sql"

$RootPath="$WorkDir\3.存储过程\后台脚本\网站库"
& sqlcmd $SqlArgs "$RootPath\删商品类.sql"

$RootPath="$WorkDir\3.存储过程\后台脚本\记录库"
& sqlcmd $SqlArgs "$RootPath\赠送会员.sql"
& sqlcmd $SqlArgs "$RootPath\赠送经验.sql"
& sqlcmd $SqlArgs "$RootPath\赠送金币.sql"
& sqlcmd $SqlArgs "$RootPath\赠送靓号.sql"

$RootPath="$WorkDir\3.存储过程\后台脚本\金币库"
& sqlcmd $SqlArgs "$RootPath\代理分成详情.sql"
& sqlcmd $SqlArgs "$RootPath\增删道具.sql"
& sqlcmd $SqlArgs "$RootPath\实卡入库.sql"
& sqlcmd $SqlArgs "$RootPath\实卡统计.sql"
& sqlcmd $SqlArgs "$RootPath\数据汇总.sql"
& sqlcmd $SqlArgs "$RootPath\新增实卡.sql"
& sqlcmd $SqlArgs "$RootPath\游戏记录.sql"
& sqlcmd $SqlArgs "$RootPath\统计记录.sql"
& sqlcmd $SqlArgs "$RootPath\赠送金币.sql"
& sqlcmd $SqlArgs "$RootPath\转账税收.sql"
& sqlcmd $SqlArgs "$RootPath\统计代理充值(手工执行).sql"
& sqlcmd $SqlArgs "$RootPath\统计玩家税收(手工执行).sql"

$RootPath="$WorkDir\4.创建作业"
& sqlcmd $SqlArgs "$RootPath\创建作业.sql"
& sqlcmd $SqlArgs "$RootPath\代理充值统计.sql"
& sqlcmd $SqlArgs "$RootPath\税收统计.sql"

Clear-Host
Write-Host "*************************************************************************"
Write-Host "                                                                         "
Write-Host "                        ！！！数据库已建立完成！！！                        "
Write-Host "                                                                         "
Write-Host "                     版权所有： 深圳市网狐科技有限公司                      "
Write-Host "*************************************************************************"
Pause
