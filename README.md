# C++ jbgz聊天室

## 首先完成服务端和客户端的联通与收发信息

大片imp WSA的问题，solve:编译选项 + -lwsock32或加静态链接库ws2_32.lib

inet_addr->vs2013更新函数，solve:关掉sdl检查即可

注意相关地址、函数参数和类型转换问题

使用本机回送地址测试

## 事件选择实现

注意！：测试事件选择线程的时候要让主线程持续运行，不然事件选择线程无法运行，会报错10061

solve:主线程while(1)，注意查错误代码10061

**258 —WSA_WAIT_TIMEOUT**
操作超时。这个Win32错误指出重叠I/O操作未在规定的时间内完成

这个卡了蛮久

1.考虑accpet重叠使用每一次都会触发新的链接，于是我将多次循环接收accept事件改为单次，没有变化

2.考虑客户端的connet请求有所触发，但是connet的事件是可以正常检测到的

3.将WAIT_TIME改为WSA_INFINITE，无果，这个会造成长时间挂起，不易过长

4.在各阶段输出报错信息，发现 WSAWaitForMultipleEvents()函数可以正常进行前几次的事件选择，无法识别到close事件，仔细想了下应该是识别服务端事件的时候造成TIMEOUT，仔细检查了下 WSAWaitForMultipleEvents（），发现参数设置有问题，改了下参数，这一块就过了