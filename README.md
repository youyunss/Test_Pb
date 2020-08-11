安装
  ```bash
  chmod 777 install.sh
  ./install.sh
  ```

运行
  ```bash
  cd bin
  ./server
  ./client
  ```

目录
```bash
.
├── bin                     #可执行文件
├── CMakeLists.txt          #cmake文件
├── install.sh              #安装文件
├── pb                      #pb协议文件与生成的头文件与源文件
│   ├── addressbook.pb.cc
│   ├── addressbook.pb.h
│   └── addressbook.proto
├── README.md
└── src                     #源码
    ├── client
    │   └── client.cpp
    └── server
        └── server.cpp
```
