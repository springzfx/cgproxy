[Unit]
Description=cgproxy service
After=network.target

[Service]
Type=simple
ExecStart=@CMAKE_INSTALL_FULL_BINDIR@/cgproxyd --execsnoop

[Install]
WantedBy=multi-user.target
