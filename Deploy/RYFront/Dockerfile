FROM mono:6.12-slim

RUN sed -i 's|deb.debian.org|archive.debian.org|g' /etc/apt/sources.list && \
    sed -i 's|security.debian.org|archive.debian.org|g' /etc/apt/sources.list && \
    echo 'Acquire::Check-Valid-Until "false";' > /etc/apt/apt.conf.d/99no-check-valid

RUN apt-get update && apt-get install -y \
    mono-fastcgi-server4 \
    libgdiplus \
    nginx \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /var/www/html

COPY _Out/pub/RYFront/ .
COPY Deploy/RYFront/nginx.conf /etc/nginx/nginx.conf

EXPOSE 80
#CMD ["mono", "/usr/lib/mono/4.5/fastcgi-mono-server4.exe", "--verbose", "--applications=/:.", "--socket=tcp:0.0.0.0:9000"]
CMD sh -c "export MONO_IOMAP=all && mono /usr/lib/mono/4.5/fastcgi-mono-server4.exe --verbose --applications=/:. --socket=tcp:127.0.0.1:9000 & nginx"
