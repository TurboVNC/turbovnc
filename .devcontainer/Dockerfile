FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install build deps + XFCE + noVNC
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential libjpeg8-dev libx11-dev libxext-dev libxtst-dev libxdamage-dev libxfixes-dev libxinerama-dev libxrandr-dev libgl1-mesa-dev libxv-dev libsm-dev libice-dev libgtk2.0-dev libssl-dev cmake autoconf automake libtool pkg-config \
    xfce4 xfce4-goodies \
    novnc websockify firefox \
    && rm -rf /var/lib/apt/lists/*

# Clone & compile official TurboVNC from this repo's source
WORKDIR /tmp
RUN git clone https://github.com/TurboVNC/turbovnc.git && \
    cd turbovnc && \
    autoreconf -fiv && \
    ./configure --prefix=/opt/TurboVNC && \
    make -j$(nproc) && \
    make install && \
    cd .. && rm -rf turbovnc

# Default password 123456
RUN mkdir -p /root/.vnc && \
    echo "123456" | /opt/TurboVNC/bin/vncpasswd -f > /root/.vnc/passwd && \
    chmod 600 /root/.vnc/passwd

# Auto-start script
RUN echo '#!/bin/bash\n\
echo "Starting TurboVNC (compiled from official repo) + noVNC..."\n\
/opt/TurboVNC/bin/vncserver :1 -geometry 1600x900 -depth 24 -securitytypes none\n\
websockify --web=/usr/share/novnc/ 6080 localhost:5901 &\n\
echo "→ Open port 6080 → password = 123456"\n\
tail -f /dev/null' > /start.sh && chmod +x /start.sh

EXPOSE 6080
CMD ["/start.sh"]
