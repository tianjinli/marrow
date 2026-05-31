FROM ubuntu:24.04

RUN apt update && apt install -y \
    supervisor \
    unixodbc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# 拷贝游戏服发布目录 (相对于 service.game-server.context 路径)
COPY _Out/pub/RYServer ./
COPY MainServer/Collocates/ServerParameter.ini ./

# 拷贝 supervisor 配置
COPY Deploy/RYServer/supervisord.conf /etc/supervisor/conf.d/supervisord.conf

# 暴露游戏服端口
EXPOSE 8600 8610 8620 8630

CMD ["/usr/bin/supervisord", "-n"]
