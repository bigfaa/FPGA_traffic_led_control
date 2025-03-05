项目功能：
设计一套智能交通信号灯控制系统，其核心在于精准控制两组共六个交通信号灯（红绿黄各两组）的亮灭状态转换，同时利用数码管直观展示路口倒计时信息。自动模式下，系统依据预设规则有条不紊地循环切换信号灯状态并精确倒计时，模拟真实交通场景下信号灯的自动调控机制，确保交通流顺畅有序。
手动模式则专为特殊交通状况而设，通过四个复位按钮灵活干预信号灯状态，操作人员按下按钮瞬间，系统即刻锁定对应路口信号灯颜色并启动倒计时，倒计时归零瞬间无缝切换至下一状态灯色，极大提升交通管理的应急响应能力与灵活性。
![image   图像](https://github.com/user-attachments/assets/cad26a11-ce54-4880-ab3f-ad59e6327e28)
![image   图像](https://github.com/user-attachments/assets/a919099d-9418-4f94-ab58-cc526592bf9f)![image   图像](https://github.com/user-attachments/assets/0b608e8a-2af2-4458-9685-a8f38a8134ab)![image   图像](https://github.com/user-attachments/assets/874c6fdc-75e8-45aa-85b1-c55d9ca2306c)

![image   图像](https://github.com/user-attachments/assets/06816c21-1b9b-48e7-b69d-d3bd976189db)

项目设计过程：
verlog代码实现的交通信号灯控制，分为自动，手动模式实现对两路信号灯控制，并同步数码管倒计时显示
该项目在modelsim软件仿真确认无误之后，在Efinity软件实现烧写到FPGA中

