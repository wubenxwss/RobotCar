#include<Reg52.h>
#include<math.h> //����ȡ����ֵ
sbit ain1 = P1 ^ 0;//����//�������������ܽ� 
sbit ain2 = P1 ^ 1;
sbit bin1 = P1 ^ 2;//�ҵ��
sbit bin2 = P1 ^ 3;
sbit pwma = P1 ^ 4;//pwm����ܽ�
sbit pwmb = P1 ^ 5;
sbit button = P1 ^ 6;
unsigned char t1 = 0;//PWM��ʱ����
unsigned char ain_pwm = 0;//pwm���ֵ
unsigned char bin_pwm = 0;
unsigned char Left_Speed = 0;//����
unsigned char Right_Speed = 0;
unsigned char tempbuf;	//�洢���յ�����Ϣ
unsigned int aMotorPulse = 0;//��¼a�������������
unsigned int bMotorPulse = 0;
unsigned int aMotorSpeed = 0;//������a����ٶȣ���λ��r/min
unsigned int bMotorSpeed = 0;
unsigned char aMotorSpeed_string[]={"A:---"};//�ַ��ٶ�
unsigned char bMotorSpeed_string[]={"B:---"};
unsigned int t2 = 0;//�������ٶȼ�ʱ����
/*******************
��������������Ӻ���
�������ٶȣ�ֵ��Χ��-100��100��
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
��������ʼ����ʱ��0
������
********************/
void Timer0Init(void)		//20΢��@11.0592MHz
{
	TMOD |= 0x01;//ģʽ1 16λ 
	TL0 = 0x23;		//���ö�ʱ��ֵ
	TH0 = 0xFF;		//���ö�ʱ��ֵ
	EA = 1;	//�����ж�
	ET0 = 1;	//����0�ж�
	TR0 = 1;  //����ʱ��
}
/*******************
��������ʱ��0���PWM
������
********************/
void IT0_interrupt() interrupt 1 
{
	TL0 = 0x23;		//���ö�ʱ��ֵ
	TH0 = 0xFF;		//���ö�ʱ��ֵ
	t1++;//PWM��ʱ����
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
		aMotorSpeed = aMotorPulse * 30 / 561 ;//24���� 
		bMotorSpeed = bMotorPulse * 30 / 561;//24���� 
		aMotorPulse = 0;
		bMotorPulse = 0;
		t2 = 0;
	}
}
/*******************
��������ʼ����������
������
********************/
void BlueteethInit()
{
	SCON = 0x50;	//����ģʽ1���������
	TMOD |= 0x20;	//T1����ģʽΪ2���Զ���װ
	PCON = 0x00;	//�����ʲ�����
	REN = 1;
	TH1 = 0xfd;		//���ò�����Ϊ9600
	TL1 = 0xfd;
	RI = 0;
	EA = 1;
	ES = 1;
	TR1 = 1;
}
/*******************
��������������
������
********************/
void Serial(void) interrupt 4
{
	tempbuf = SBUF;
	RI = 0;	//����־����
	SBUF = tempbuf;	//�����ݷ��ص��ֻ��ˣ������ֻ��鿴���͵�����
	while (!TI);
	TI = 0;	//д��־����
}
/*******************
������ͨ�����ڿ��Ƴ�ǰ������
������
********************/
void ControlCar() {
	if (tempbuf == 'q') {//ǰ��
		Run(Left_Speed, Right_Speed);
	}
	if (tempbuf == 'h') {//����
		Run(-Left_Speed, -Right_Speed);
	}
	if (tempbuf == 'z') {//��ת
		Run(-Left_Speed, Right_Speed);
	}
	if (tempbuf == 'y') {//��ת
		Run(Left_Speed, -Right_Speed);
	}
	if (tempbuf == 't') {//ͣ��
		Run(0, 0);
	}
	if (tempbuf == '9') {//ͣ��
		Left_Speed = 90;
		Right_Speed = 90;
	}
	if (tempbuf == '5') {//ͣ��
		Left_Speed = 50;
		Right_Speed = 50;
	}

}
/*******************
��������ʱ����
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
�����������ⲿ�ж�
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
�������������
********************/
void EX0_interrupt() interrupt 0
{
	aMotorPulse++;//a���������������
}
/*******************
�������������
********************/
void EX1_interrupt() interrupt 2
{
	bMotorPulse++;//a���������������
}

/*******************
�����������ֽ�
********************/
void SendByte(unsigned char str)
{
	ES =0;//���رջ��������
	if (TI == 0)
	{
		SBUF = str;
		while (TI == 0);
		TI = 0;
	}
	ES =1;
}

/*******************
�����������ֽ�
********************/
void SendByte2(unsigned char str)
{
	//ES =0;//���رջ��������
	if (TI == 0)
	{
		SBUF = str;
		while (TI == 0);
		TI = 0;
	}
	//ES =1;
}
/*******************
�����������ַ���
********************/
void SendString(unsigned char *s)
{
	ES =0;//���رջ��������
	while (*s != 0)
	{
		SendByte2(*s++);
	}
	ES =1;
}
void Format_DateTime(unsigned char d,unsigned char *a)
{
	a[0]=d/100+'0';  	//ȡ��λ
 	a[1]=d/10%10+'0';  	//ȡʮλ
	a[2]=d%10+'0'; 		//ȡ��λ
}
/*******************
������������
********************/
void main()
{
	unsigned char P10 = 0;
	unsigned char i_send = 0;
	Timer0Init();//��ʱ����ʼ����20΢��@11.0592MHz//
	initEX();//�ⲿ�жϳ�ʼ��
	BlueteethInit();//�������ڳ�ʼ��
	Run(0, 0);
	button = 1;//P2.6��1
	Left_Speed = 50;
	Right_Speed = 50;
	while (1)
	{
		if (P10 == 0)//���Ե���Ƿ�����
		{
			if (button == 0)//��⿪��P1.6����
			{
				Run(-99, -99);//�õ��ֱ��
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
	
		if (P10 == 1) //���Դ����Ƿ�����,���Ȱ�ֹͣ��ť��ʼ
		{
			if (tempbuf == 't')
			{
				Run(99, 99);
				DelayMS(500);
				Run(0, 0);
				P10 = 2;
			}
		}
		
		if (P10 == 2)//�������������Ƿ�����
		{
			SendByte('A');
			SendByte('B');
			SendByte('2');
			SendByte('0');
			SendString("Ready");
			P10 = 3;	
			
		}
		if (P10 == 3)//�����ٶȷ����Ƿ�����
		{	
			
			aMotorSpeed = 150; //���Թ̶��ٶ�
			bMotorSpeed = 201; 
			Format_DateTime(aMotorSpeed,aMotorSpeed_string+2);//�仯�ѳ����α�Ϊ�ַ���
			Format_DateTime(bMotorSpeed,bMotorSpeed_string+2);
			SendString(aMotorSpeed_string);//�����ٶ��ַ���
			SendString(bMotorSpeed_string);
			Run(50,50);
			DelayMS(5000);
			Format_DateTime(aMotorSpeed,aMotorSpeed_string+2);//����ʵʱ�ٶ��ַ���
			Format_DateTime(bMotorSpeed,bMotorSpeed_string+2);
			SendString(aMotorSpeed_string);
			SendString(bMotorSpeed_string);
			Run(0,0);
			P10 = 4;
			
		}
		if (P10 == 4) //��ʽ��ʼ����
		{
			ControlCar();//��������ǰ��
			//P10 = 2;
		}

	}
}