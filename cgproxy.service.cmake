[Unit]
Description=cgproxy service
After=network.target network-online.target

[Service]
Type=simple
ExecStart=@CMAKE_INSTALL_FULL_BINDIR@/cgproxyd --execsnoop
Restart=always

[Install]
WantedBy=multi-user.target
