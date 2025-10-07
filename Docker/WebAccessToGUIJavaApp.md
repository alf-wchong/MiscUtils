# Java GUI Application with VNC + noVNC - Amazon Linux 2/RHEL/Fedora Version

## Complete Implementation Guide for Red Hat-based Distributions

This guide provides a complete solution for running a Java Swing GUI application in Docker with VNC + noVNC access on Amazon Linux 2, RHEL, or Fedora, deployed on AWS EKS.

## 1. Dockerfile for Amazon Linux 2

Create a `Dockerfile-amazonlinux2` for your Java GUI application:

```dockerfile
# Use Amazon Linux 2 as base image
FROM amazonlinux:2

# Install EPEL repository for additional packages
RUN yum install -y epel-release amazon-linux-extras

# Enable GUI packages
RUN amazon-linux-extras install -y mate-desktop1.x

# Install required packages
RUN yum update -y && yum install -y \
    java-11-openjdk \
    java-11-openjdk-devel \
    tigervnc-server \
    firefox \
    xorg-x11-server-Xvfb \
    xorg-x11-fonts-Type1 \
    xorg-x11-fonts-misc \
    supervisor \
    python3 \
    python3-pip \
    wget \
    curl \
    net-tools \
    && yum clean all

# Install noVNC and websockify via pip
RUN pip3 install numpy websockify

# Download and setup noVNC
RUN cd /opt && \
    wget -qO- https://github.com/novnc/noVNC/archive/v1.3.0.tar.gz | tar xz && \
    mv noVNC-1.3.0 noVNC && \
    ln -s /opt/noVNC/vnc.html /opt/noVNC/index.html

# Create VNC user and directories
RUN useradd -m vncuser && \
    mkdir -p /home/vncuser/.vnc && \
    chown -R vncuser:vncuser /home/vncuser

# Set VNC password
USER vncuser
RUN echo "vncpassword" | vncpasswd -f > /home/vncuser/.vnc/passwd && \
    chmod 600 /home/vncuser/.vnc/passwd

# Create VNC startup script for MATE desktop
RUN echo '#!/bin/bash\n\
export USER=vncuser\n\
export HOME=/home/vncuser\n\
mate-session &\n\
' > /home/vncuser/.vnc/xstartup && \
    chmod +x /home/vncuser/.vnc/xstartup

# Switch back to root for application setup
USER root

# Copy your Java application
COPY your-app.jar /app/
COPY swing-layout-1.0.3.jar /app/

# Set working directory
WORKDIR /app

# Create supervisor configuration
RUN echo '[supervisord]\n\
nodaemon=true\n\
user=root\n\
\n\
[program:xvfb]\n\
command=/usr/bin/Xvfb :1 -screen 0 1024x768x24 -ac +extension GLX +render -noreset\n\
user=vncuser\n\
autorestart=true\n\
priority=100\n\
\n\
[program:vncserver]\n\
command=/usr/bin/vncserver :1 -geometry 1024x768 -depth 24 -fg -localhost no\n\
user=vncuser\n\
environment=DISPLAY=":1",USER="vncuser",HOME="/home/vncuser"\n\
autorestart=true\n\
priority=200\n\
\n\
[program:novnc]\n\
command=/usr/local/bin/websockify --web /opt/noVNC 6901 localhost:5901\n\
autorestart=true\n\
priority=300\n\
\n\
[program:java-app]\n\
command=java -cp your-app.jar:swing-layout-1.0.3.jar your.main.ClassName\n\
environment=DISPLAY=":1"\n\
user=vncuser\n\
directory=/app\n\
autorestart=true\n\
startretries=10\n\
startsecs=30\n\
priority=400\n\
' > /etc/supervisord.conf

# Expose VNC and noVNC ports
EXPOSE 5901 6901

# Set display environment
ENV DISPLAY=:1

# Start supervisor
CMD ["/usr/bin/supervisord", "-c", "/etc/supervisord.conf"]
```

## 2. Dockerfile for Amazon Linux 2023

Create a `Dockerfile-amazonlinux2023` for your Java GUI application:

```dockerfile
# Use Amazon Linux 2023 as base image
FROM amazonlinux:2023

# Install required packages using dnf
RUN dnf update -y && dnf install -y \
    java-11-openjdk \
    java-11-openjdk-devel \
    tigervnc-server \
    firefox \
    xorg-x11-server-Xvfb \
    xorg-x11-fonts-Type1 \
    xorg-x11-fonts-misc \
    python3 \
    python3-pip \
    supervisor \
    wget \
    curl \
    net-tools \
    @xfce-desktop-environment \
    && dnf clean all

# Install noVNC and websockify via pip
RUN pip3 install websockify

# Download and setup noVNC
RUN cd /opt && \
    wget -qO- https://github.com/novnc/noVNC/archive/v1.3.0.tar.gz | tar xz && \
    mv noVNC-1.3.0 noVNC && \
    ln -s /opt/noVNC/vnc.html /opt/noVNC/index.html

# Create VNC user and directories
RUN useradd -m vncuser && \
    mkdir -p /home/vncuser/.vnc && \
    chown -R vncuser:vncuser /home/vncuser

# Set VNC password
USER vncuser
RUN echo "vncpassword" | vncpasswd -f > /home/vncuser/.vnc/passwd && \
    chmod 600 /home/vncuser/.vnc/passwd

# Create VNC startup script for XFCE desktop
RUN echo '#!/bin/bash\n\
export USER=vncuser\n\
export HOME=/home/vncuser\n\
startxfce4 &\n\
' > /home/vncuser/.vnc/xstartup && \
    chmod +x /home/vncuser/.vnc/xstartup

# Switch back to root for application setup
USER root

# Copy your Java application
COPY your-app.jar /app/
COPY swing-layout-1.0.3.jar /app/

# Set working directory
WORKDIR /app

# Create supervisor configuration (same as AL2 version)
RUN echo '[supervisord]\n\
nodaemon=true\n\
user=root\n\
\n\
[program:xvfb]\n\
command=/usr/bin/Xvfb :1 -screen 0 1024x768x24 -ac +extension GLX +render -noreset\n\
user=vncuser\n\
autorestart=true\n\
priority=100\n\
\n\
[program:vncserver]\n\
command=/usr/bin/vncserver :1 -geometry 1024x768 -depth 24 -fg -localhost no\n\
user=vncuser\n\
environment=DISPLAY=":1",USER="vncuser",HOME="/home/vncuser"\n\
autorestart=true\n\
priority=200\n\
\n\
[program:novnc]\n\
command=/usr/local/bin/websockify --web /opt/noVNC 6901 localhost:5901\n\
autorestart=true\n\
priority=300\n\
\n\
[program:java-app]\n\
command=java -cp your-app.jar:swing-layout-1.0.3.jar your.main.ClassName\n\
environment=DISPLAY=":1"\n\
user=vncuser\n\
directory=/app\n\
autorestart=true\n\
startretries=10\n\
startsecs=30\n\
priority=400\n\
' > /etc/supervisord.conf

# Expose VNC and noVNC ports
EXPOSE 5901 6901

# Set display environment
ENV DISPLAY=:1

# Start supervisor
CMD ["/usr/bin/supervisord", "-c", "/etc/supervisord.conf"]
```

## 3. Dockerfile for Fedora

Create a `Dockerfile-fedora` for your Java GUI application:

```dockerfile
# Use Fedora as base image
FROM fedora:39

# Install required packages using dnf
RUN dnf update -y && dnf install -y \
    java-11-openjdk \
    java-11-openjdk-devel \
    tigervnc-server \
    firefox \
    xorg-x11-server-Xvfb \
    xorg-x11-fonts-Type1 \
    xorg-x11-fonts-misc \
    python3 \
    python3-pip \
    supervisor \
    wget \
    curl \
    net-tools \
    @xfce-desktop-environment \
    && dnf clean all

# Install noVNC and websockify via pip
RUN pip3 install websockify

# Download and setup noVNC
RUN cd /opt && \
    wget -qO- https://github.com/novnc/noVNC/archive/v1.3.0.tar.gz | tar xz && \
    mv noVNC-1.3.0 noVNC && \
    ln -s /opt/noVNC/vnc.html /opt/noVNC/index.html

# Create VNC user and directories
RUN useradd -m vncuser && \
    mkdir -p /home/vncuser/.vnc && \
    chown -R vncuser:vncuser /home/vncuser

# Set VNC password
USER vncuser
RUN echo "vncpassword" | vncpasswd -f > /home/vncuser/.vnc/passwd && \
    chmod 600 /home/vncuser/.vnc/passwd

# Create VNC startup script for XFCE desktop
RUN echo '#!/bin/bash\n\
export USER=vncuser\n\
export HOME=/home/vncuser\n\
startxfce4 &\n\
' > /home/vncuser/.vnc/xstartup && \
    chmod +x /home/vncuser/.vnc/xstartup

# Switch back to root for application setup
USER root

# Copy your Java application
COPY your-app.jar /app/
COPY swing-layout-1.0.3.jar /app/

# Set working directory
WORKDIR /app

# Create supervisor configuration (same as above)
RUN echo '[supervisord]\n\
nodaemon=true\n\
user=root\n\
\n\
[program:xvfb]\n\
command=/usr/bin/Xvfb :1 -screen 0 1024x768x24 -ac +extension GLX +render -noreset\n\
user=vncuser\n\
autorestart=true\n\
priority=100\n\
\n\
[program:vncserver]\n\
command=/usr/bin/vncserver :1 -geometry 1024x768 -depth 24 -fg -localhost no\n\
user=vncuser\n\
environment=DISPLAY=":1",USER="vncuser",HOME="/home/vncuser"\n\
autorestart=true\n\
priority=200\n\
\n\
[program:novnc]\n\
command=/usr/local/bin/websockify --web /opt/noVNC 6901 localhost:5901\n\
autorestart=true\n\
priority=300\n\
\n\
[program:java-app]\n\
command=java -cp your-app.jar:swing-layout-1.0.3.jar your.main.ClassName\n\
environment=DISPLAY=":1"\n\
user=vncuser\n\
directory=/app\n\
autorestart=true\n\
startretries=10\n\
startsecs=30\n\
priority=400\n\
' > /etc/supervisord.conf

# Expose VNC and noVNC ports
EXPOSE 5901 6901

# Set display environment
ENV DISPLAY=:1

# Start supervisor
CMD ["/usr/bin/supervisord", "-c", "/etc/supervisord.conf"]
```

## 4. Dockerfile for RHEL 8/9

Create a `Dockerfile-rhel` for your Java GUI application:

```dockerfile
# Use RHEL 9 as base image (requires valid Red Hat subscription)
FROM registry.redhat.io/rhel9/rhel:latest

# Install required packages using dnf
RUN dnf update -y && dnf install -y \
    java-11-openjdk \
    java-11-openjdk-devel \
    tigervnc-server \
    firefox \
    xorg-x11-server-Xvfb \
    xorg-x11-fonts-Type1 \
    xorg-x11-fonts-misc \
    python3 \
    python3-pip \
    supervisor \
    wget \
    curl \
    net-tools \
    @workstation-product-environment \
    && dnf clean all

# Install noVNC and websockify via pip
RUN pip3 install websockify

# Download and setup noVNC
RUN cd /opt && \
    wget -qO- https://github.com/novnc/noVNC/archive/v1.3.0.tar.gz | tar xz && \
    mv noVNC-1.3.0 noVNC && \
    ln -s /opt/noVNC/vnc.html /opt/noVNC/index.html

# Create VNC user and directories
RUN useradd -m vncuser && \
    mkdir -p /home/vncuser/.vnc && \
    chown -R vncuser:vncuser /home/vncuser

# Set VNC password
USER vncuser
RUN echo "vncpassword" | vncpasswd -f > /home/vncuser/.vnc/passwd && \
    chmod 600 /home/vncuser/.vnc/passwd

# Create VNC startup script for GNOME desktop
RUN echo '#!/bin/bash\n\
export USER=vncuser\n\
export HOME=/home/vncuser\n\
gnome-session &\n\
' > /home/vncuser/.vnc/xstartup && \
    chmod +x /home/vncuser/.vnc/xstartup

# Switch back to root for application setup
USER root

# Copy your Java application
COPY your-app.jar /app/
COPY swing-layout-1.0.3.jar /app/

# Set working directory
WORKDIR /app

# Create supervisor configuration (same as above)
RUN echo '[supervisord]\n\
nodaemon=true\n\
user=root\n\
\n\
[program:xvfb]\n\
command=/usr/bin/Xvfb :1 -screen 0 1024x768x24 -ac +extension GLX +render -noreset\n\
user=vncuser\n\
autorestart=true\n\
priority=100\n\
\n\
[program:vncserver]\n\
command=/usr/bin/vncserver :1 -geometry 1024x768 -depth 24 -fg -localhost no\n\
user=vncuser\n\
environment=DISPLAY=":1",USER="vncuser",HOME="/home/vncuser"\n\
autorestart=true\n\
priority=200\n\
\n\
[program:novnc]\n\
command=/usr/local/bin/websockify --web /opt/noVNC 6901 localhost:5901\n\
autorestart=true\n\
priority=300\n\
\n\
[program:java-app]\n\
command=java -cp your-app.jar:swing-layout-1.0.3.jar your.main.ClassName\n\
environment=DISPLAY=":1"\n\
user=vncuser\n\
directory=/app\n\
autorestart=true\n\
startretries=10\n\
startsecs=30\n\
priority=400\n\
' > /etc/supervisord.conf

# Expose VNC and noVNC ports
EXPOSE 5901 6901

# Set display environment
ENV DISPLAY=:1

# Start supervisor
CMD ["/usr/bin/supervisord", "-c", "/etc/supervisord.conf"]
```

## 5. Build and Deploy Instructions

Choose the appropriate Dockerfile for your preferred distribution:

```bash
# For Amazon Linux 2
docker build -f Dockerfile-amazonlinux2 -t your-registry/java-gui-app:al2 .

# For Amazon Linux 2023
docker build -f Dockerfile-amazonlinux2023 -t your-registry/java-gui-app:al2023 .

# For Fedora
docker build -f Dockerfile-fedora -t your-registry/java-gui-app:fedora .

# For RHEL
docker build -f Dockerfile-rhel -t your-registry/java-gui-app:rhel .

# Push to registry
docker push your-registry/java-gui-app:al2023

# Deploy to EKS (same Kubernetes manifests as before)
kubectl apply -f k8s-deployment.yaml
kubectl apply -f k8s-service.yaml
```

## 6. Customization Options

### VNC Password
```dockerfile
# Change "vncpassword" to your desired password
RUN echo "your-secure-password" | vncpasswd -f > /home/vncuser/.vnc/passwd
```

### Screen Resolution
```dockerfile
# Modify both Xvfb and vncserver commands
command=/usr/bin/Xvfb :1 -screen 0 1920x1080x24 -ac +extension GLX +render -noreset
command=/usr/bin/vncserver :1 -geometry 1920x1080 -depth 24 -fg -localhost no
```

### Java Application Arguments
```dockerfile
# Add JVM arguments as needed
command=java -Xmx2g -Djava.awt.headless=false -cp your-app.jar:swing-layout-1.0.3.jar your.main.ClassName
```

## 7. Recommended Choice

For your use case, I recommend **Amazon Linux 2023** because:

1. **Modern Package Manager**: Uses `dnf` which is faster and more reliable
2. **Long-term Support**: Actively maintained by AWS
3. **Container Optimized**: Designed for containerized workloads
4. **XFCE Desktop**: Lightweight but feature-rich desktop environment
5. **AWS Integration**: Best compatibility with EKS and other AWS services

## 8. Testing Locally

Before deploying to EKS, test locally:

```bash
# Build the image
docker build -f Dockerfile-amazonlinux2023 -t java-gui-test .

# Run locally
docker run -p 6901:6901 -p 5901:5901 java-gui-test

# Access via browser
# http://localhost:6901/vnc.html
```

This Red Hat-based implementation provides the same functionality while using the package managers and desktop environments you prefer!
