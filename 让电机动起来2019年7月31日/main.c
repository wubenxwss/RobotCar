#include<Reg52.h>
#include<math.h> //用于取绝对值
sbit ain1 = P1 ^ 0;//左电机//定义输出到电机管脚 
sbit ain2 = P1 ^ 1;
sbit bin1 = P1 ^ 2;//右电机
sbit bin2 = P1 ^ 3;
sbit pwma = P1 ^ 4;//pwm输出管脚
sbit pwmb = P1 ^ 5;
sbit button = P1 ^ 6;
unsigned char t1 = 0;//PWM计时因子
unsigned char ain_pwm = 0;//pwm输出值
unsigned char bin_pwm = 0;
unsigned char Left_Speed = 0;//车速
unsigned char Right_Speed = 0;
unsigned char tempbuf;	//存储接收到的信息
unsigned int aMotorPulse = 0;//记录a电机编码器脉冲
unsigned int bMotorPulse = 0;
unsigned int aMotorSpeed = 0;//编码器a电机速度，单位：r/min
unsigned int bMotorSpeed = 0;
unsigned char aMotorSpeed_string[]={"A:---"};//字符速度
unsigned char bMotorSpeed_string[]={"B:---"};
unsigned int t2 = 0;//编码盘速度计时因子
/*******************
函数：电机驱动子函数
参数：速度，值范围（-100至100）
********************/
void Run(char left_speed, char right_speed)
{
	if (left_speed < 0) {
		ain1 = 0;
		ain2 = 1;
	}
	if (left_speed > 0) {
		ain1 = 1;
		ain2 = 0;
	}
	if (left_speed == 0) {
		ain1 = 0;
		ain2 = 0;
	}
	ain_pwm = abs(left_speed);
	if (right_speed < 0) {
		bin1 = 0;
		bin2 = 1;
	}
	if (right_speed > 0) {
		bin1 = 1;
		bin2 = 0;
	}
	if (right_speed == 0) {
		bin1 = 0;
		bin2 = 0;
	}
	bin_pwm = abs(right_speed);
}
/*******************
函数：初始化定时器0
参数：
********************/
void Timer0Init(void)		//20微秒@11.0592MHz
{
	TMOD |= 0x01;//模式1 16位 
	TL0 = 0x23;		//设置定时初值
	TH0 = 0xFF;		//设置定时初值
	EA = 1;	//开总中断
	ET0 = 1;	//开定0中断
	TR0 = 1;  //开定时器
}
/*******************
函数：定时器0输出PWM
参数：
********************/
void IT0_interrupt() interrupt 1 
{
	TL0 = 0x23;		//设置定时初值
	TH0 = 0xFF;		//设置定时初值
	t1++;//PWM计时因子
	if (t1 == ain_pwm) {
		pwma = ~pwma;
	}
	if (t1 == bin_pwm) {
		pwmb = ~pwmb;
	}
	else if (t1 == 100) {
		t1 = 0;
		pwma = 1;
		pwmb = 1;
	}

	t2++; //
	if (t2 == 50*500) {//0.5s	  //60
		aMotorSpeed = aMotorPulse * 30 / 561 ;//24脉冲 
		bMotorSpeed = bMotorPulse * 30 / 561;//24脉冲 
		aMotorPulse = 0;
		bMotorPulse = 0;
		t2 = 0;
	}
}
/*******************
函数：初始化蓝牙串口
参数：
********************/
void BlueteethInit()
{
	SCON = 0x50;	//串口模式1，允许接收
	TMOD |= 0x20;	//T1工作模式为2，自动重装
	PCON = 0x00;	//波特率不倍增
	REN = 1;
	TH1 = 0xfd;		//设置波特率为9600
	TL1 = 0xfd;
	RI = 0;
	EA = 1;
	ES = 1;
	TR1 = 1;
}
/*******************
函数：蓝牙串口
参数：
********************/
void Serial(void) interrupt 4
{
	tempbuf = SBUF;
	RI = 0;	//读标志清零
	SBUF = tempbuf;	//将内容返回到手机端，可在手机查看发送的内容
	while (!TI);
	TI = 0;	//写标志清零
}
/*******************
函数：通过串口控制车前进后退
参数：
********************/
void ControlCar() {
	if (tempbuf == 'q') {//前进
		Run(Left_Speed, Right_Speed);
	}
	if (tempbuf == 'h') {//后退
		Run(-Left_Speed, -Right_Speed);
	}
	if (tempbuf == 'z') {//左转
		Run(-Left_Speed, Right_Speed);
	}
	if (tempbuf == 'y') {//右转
		Run(Left_Speed, -Right_Speed);
	}
	if (tempbuf == 't') {//停车
		Run(0, 0);
	}
	if (tempbuf == '9') {//停车
		Left_Speed = 90;
		Right_Speed = 90;
	}
	if (tempbuf == '5') {//停车
		Left_Speed = 50;
		Right_Speed = 50;
	}

}
/*******************
函数：延时毫米
********************/
void DelayMS(unsigned int ms)
{
	unsigned char i;
	while (ms--)
	{
		for (i = 0; i < 120; i++);
	}
}
/*******************
函数：开启外部中断
********************/
void initEX()
{
	//EA = 1;
	EX0 = 1;
	IT0 = 1;
	EX1 = 1;
	IT1 = 1;
}

/*******************
函数：脉冲计数
********************/
void EX0_interrupt() interrupt 0
{
	aMotorPulse++;//a电机编码器计数加
}
/*******************
函数：脉冲计数
********************/
void EX1_interrupt() interrupt 2
{
	bMotorPulse++;//a电机编码器计数加
}

/*******************
函数：发送字节
********************/
void SendByte(unsigned char str)
{
	ES =0;//不关闭会产生干扰
	if (TI == 0)
	{
		SBUF = str;
		while (TI == 0);
		TI = 0;
	}
	ES =1;
}

/*******************
函数：发送字节
********************/
void SendByte2(unsigned char str)
{
	//ES =0;//不关闭会产生干扰
	if (TI == 0)
	{
		SBUF = str;
		while (TI == 0);
		TI = 0;
	}
	//ES =1;
}
/*******************
函数：发送字符串
********************/
void SendString(unsigned char *s)
{
	ES =0;//不关闭会产生干扰
	while (*s != 0)
	{
		SendByte2(*s++);
	}
	ES =1;
}
void Format_DateTime(unsigned char d,unsigned char *a)
{
	a[0]=d/100+'0';  	//取百位
 	a[1]=d/10%10+'0';  	//取十位
	a[2]=d%10+'0'; 		//取个位
}
/*******************
函数：主函数
********************/
void main()
{
	unsigned char P10 = 0;
	unsigned char i_send = 0;
	Timer0Init();//定时器初始化，20微秒@11.0592MHz//
	initEX();//外部中断初始化
	BlueteethInit();//蓝牙串口初始化
	Run(0, 0);
	button = 1;//P2.6置1
	Left_Speed = 50;
	Right_Speed = 50;
	while (1)
	{
		if (P10 == 0)//测试电机是否运行
		{
			if (button == 0)//检测开关P1.6按下
			{
				Run(-99, -99);//让电机直行
				DelayMS(500);
				Run(-Left_Speed, Right_Speed);
				DelayMS(500);
				ain1 = 0;
				ain2 = 0;
				bin1 = 0;
				bin2 = 0;
				P10 = 1;
			}
			else {
				ain1 = 0;
				ain2 = 0;
				bin1 = 0;
				bin2 = 0;
			}
		}
	
		if (P10 == 1) //测试串口是否运行,即先按停止按钮开始
		{
			if (tempbuf == 't')
			{
				Run(99, 99);
				DelayMS(500);
				Run(0, 0);
				P10 = 2;
			}
		}
		
		if (P10 == 2)//测试蓝牙反馈是否正常
		{
			SendByte('A');
			SendByte('B');
			SendByte('2');
			SendByte('0');
			SendString("Ready");
			P10 = 3;	
			
		}
		if (P10 == 3)//测试速度反馈是否正常
		{	
			
			aMotorSpeed = 150; //测试固定速度
			bMotorSpeed = 201; 
			Format_DateTime(aMotorSpeed,aMotorSpeed_string+2);//变化把长整形变为字符串
			Format_DateTime(bMotorSpeed,bMotorSpeed_string+2);
			SendString(aMotorSpeed_string);//发送速度字符串
			SendString(bMotorSpeed_string);
			Run(50,50);
			DelayMS(5000);
			Format_DateTime(aMotorSpeed,aMotorSpeed_string+2);//发送实时速度字符串
			Format_DateTime(bMotorSpeed,bMotorSpeed_string+2);
			SendString(aMotorSpeed_string);
			SendString(bMotorSpeed_string);
			Run(0,0);
			P10 = 4;
			
		}
		if (P10 == 4) //正式开始运行
		{
			ControlCar();//蓝牙控制前进
			//P10 = 2;
		}

	}
}