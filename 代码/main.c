////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2013-2023 Efinix Inc. All rights reserved.              
//
// This   document  contains  proprietary information  which   is        
// protected by  copyright. All rights  are reserved.  This notice       
// refers to original work by Efinix, Inc. which may be derivitive       
// of other work distributed under license of the authors.  In the       
// case of derivative work, nothing in this notice overrides the         
// original author's license agreement.  Where applicable, the           
// original license agreement is included in it's original               
// unmodified form immediately below this header.                        
//                                                                       
// WARRANTY DISCLAIMER.                                                  
//     THE  DESIGN, CODE, OR INFORMATION ARE PROVIDED “AS IS” AND        
//     EFINIX MAKES NO WARRANTIES, EXPRESS OR IMPLIED WITH               
//     RESPECT THERETO, AND EXPRESSLY DISCLAIMS ANY IMPLIED WARRANTIES,  
//     INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF          
//     MERCHANTABILITY, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR    
//     PURPOSE.  SOME STATES DO NOT ALLOW EXCLUSIONS OF AN IMPLIED       
//     WARRANTY, SO THIS DISCLAIMER MAY NOT APPLY TO LICENSEE.           
//                                                                       
// LIMITATION OF LIABILITY.                                              
//     NOTWITHSTANDING ANYTHING TO THE CONTRARY, EXCEPT FOR BODILY       
//     INJURY, EFINIX SHALL NOT BE LIABLE WITH RESPECT TO ANY SUBJECT    
//     MATTER OF THIS AGREEMENT UNDER TORT, CONTRACT, STRICT LIABILITY   
//     OR ANY OTHER LEGAL OR EQUITABLE THEORY (I) FOR ANY INDIRECT,      
//     SPECIAL, INCIDENTAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES OF ANY    
//     CHARACTER INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF      
//     GOODWILL, DATA OR PROFIT, WORK STOPPAGE, OR COMPUTER FAILURE OR   
//     MALFUNCTION, OR IN ANY EVENT (II) FOR ANY AMOUNT IN EXCESS, IN    
//     THE AGGREGATE, OF THE FEE PAID BY LICENSEE TO EFINIX HEREUNDER    
//     (OR, IF THE FEE HAS BEEN WAIVED, $100), EVEN IF EFINIX SHALL HAVE 
//     BEEN INFORMED OF THE POSSIBILITY OF SUCH DAMAGES.  SOME STATES DO 
//     NOT ALLOW THE EXCLUSION OR LIMITATION OF INCIDENTAL OR            
//     CONSEQUENTIAL DAMAGES, SO THIS LIMITATION AND EXCLUSION MAY NOT   
//     APPLY TO LICENSEE.                                                
//
////////////////////////////////////////////////////////////////////////////////

/******************************************************************************
*
* @file main.c: GpioDemo (onBoard LED Blinking Demo)
*
* @brief This demo performs onboard LED blinking through GPIO controller 
*        and allow external interrupt by user. 
*
******************************************************************************/
#include <stdint.h>
#include "bsp.h"
#include "riscv.h"
#include "gpio.h"
#include "clint.h"
#include "plic.h"

#ifdef SIM //Faster timer tick in simulation to avoid having to wait too long
    #define LOOP_UDELAY 100   //时间是100us
#else
    #define LOOP_UDELAY 20000 //时间是0.2s
#endif

#ifdef SYSTEM_GPIO_0_IO_CTRL
    
    #define GPIO0       SYSTEM_GPIO_0_IO_CTRL

// 定义一个常量用于表示呼吸灯的周期（单位：毫秒），可根据实际需求调整
#define BREATHING_PERIOD_MS 20

// 定义一个常量用于表示PWM的分辨率（这里假设为8位，取值范围0 - 255）
#define PWM_RESOLUTION 10




void init();
void main();
void trap();
void crash();
void trap_entry();
void externalInterrupt();




//*******************************************
// 定义一个结构体用于存储PWM相关信息
typedef struct {
    uint32_t dutyCycle;  // 占空比，范围0 - (2^PWM_RESOLUTION - 1)
    uint32_t period;     // PWM周期（以某个时间单位计量，这里暂未精确到具体单位）
    uint32_t counter;    // 用于计数，在每个周期内递增
} PWMInfo;

// 全局变量，用于存储当前PWM信息
PWMInfo pwmInfo;
// 函数用于初始化PWM相关设置
void pwm_init() {
    pwmInfo.dutyCycle = 0;
    pwmInfo.period = BREATHING_PERIOD_MS * (1 << PWM_RESOLUTION);
    // 根据呼吸灯周期和分辨率计算总周期计数 pwmInfo.period = BREATHING_PERIOD_MS * 256
    pwmInfo.counter = 0;
}

// 函数用于更新PWM输出，根据当前占空比设置GPIO输出
void pwm_update_output(uint32_t gpioPort) {
    if (pwmInfo.counter < pwmInfo.dutyCycle) {
        gpio_setOutput(GPIO0, 0xFF);  // 假设高电平点亮LED，根据实际情况调整
    } else {
        gpio_setOutput(GPIO0, 0x00);
    }
}

// 函数用于递增PWM的占空比，实现呼吸灯的渐亮效果
void pwm_increase_duty_cycle() {
	// 255
    if (pwmInfo.dutyCycle < (1 << PWM_RESOLUTION) - 1) {
        pwmInfo.dutyCycle++;
    }
}

// 函数用于递减PWM的占空比，实现呼吸灯的渐暗效果
void pwm_decrease_duty_cycle() {
    if (pwmInfo.dutyCycle > 0) {
        pwmInfo.dutyCycle--;
    }
}

//***********************************************************






/******************************************************************************
*
* @brief This function initializes GPIO interrupts and enables external interrupts
*        by setting up the machine trap vector. 
*
******************************************************************************/
void init(){

    plic_set_threshold(BSP_PLIC, BSP_PLIC_CPU_0, 0); 
    plic_set_enable(BSP_PLIC, BSP_PLIC_CPU_0, SYSTEM_PLIC_SYSTEM_GPIO_0_IO_INTERRUPTS_0, 1);
    plic_set_priority(BSP_PLIC, SYSTEM_PLIC_SYSTEM_GPIO_0_IO_INTERRUPTS_0, 1);
    gpio_setInterruptRiseEnable(GPIO0, 1); 
    csr_write(mtvec, trap_entry); 
    //Enable external interrupts
    csr_set(mie, MIE_MEIE); 
    csr_write(mstatus, csr_read(mstatus) | MSTATUS_MPP | MSTATUS_MIE);
}


/******************************************************************************
*
* @brief This function handles exceptions and interrupts in the system.
*
* @note It is called by the trap_entry function on both exceptions and interrupts 
* 		events. If the cause of the trap is an interrupt, it checks the cause of 
* 		the interrupt and calls corresponding interrupt handler functions. If 
* 		the cause is an exception or an unhandled interrupt, it calls a 
*		crash function to handle the error.
*
******************************************************************************/
void trap(){
    int32_t mcause = csr_read(mcause);
    int32_t interrupt = mcause < 0;    //Interrupt if true, exception if false
    int32_t cause     = mcause & 0xF;
    if(interrupt){
        switch(cause){
        case CAUSE_MACHINE_EXTERNAL: externalInterrupt(); break;
        default: crash(); break;
        }
    } else {
        crash();
    }
}


/******************************************************************************
*
* @brief This function handles external interrupts. It checks for pending
*        interrupts and processes them accordingly. If an interrupt from
*        GPIO 0 is detected, it prints a message indicating the interrupt
*        source. If an interrupt from an unknown source is detected, it 
*        calls a crash function to handle the error.
*
******************************************************************************/
void externalInterrupt(){
    uint32_t claim;
    //While there is pending interrupts
    while(claim = plic_claim(BSP_PLIC, BSP_PLIC_CPU_0)){
        switch(claim){
        case SYSTEM_PLIC_SYSTEM_GPIO_0_IO_INTERRUPTS_0: bsp_printf("gpio 0 interrupt routine \r\n"); break;
        default: crash(); break;
        }
        //unmask the claimed interrupt
        plic_release(BSP_PLIC, BSP_PLIC_CPU_0, claim); 
    }
}


/******************************************************************************
*
* @brief This function handles the system crash scenario by printing a crash message
* 		 and entering an infinite loop.
*
******************************************************************************/
void crash(){
    bsp_printf("\r\n*** CRASH ***\r\n");
    while(1);
}

/******************************************************************************
*
* @brief This main function initializes the board, configures GPIO 0 to control onboard
*        LEDs, and then sets up a loop to blink the LEDs. After that, it
*        demonstrates GPIO 0 interrupt handling by instructing the user to press
*        and release various onboard buttons (sw4, sw6, sw7) corresponding to
*        different timer intervals.
*
******************************************************************************/

void main() {
    bsp_init();
    bsp_printf("gpio 0 demo! \r\n");
    bsp_printf("onboard LEDs blinking \r\n");
//    configure 4 bits gpio 0
//     流水灯效果
    gpio_setOutputEnable(GPIO0, 0xFF);
        gpio_setOutput(GPIO0, 0xFF);//
while(1){
	for(int i=0;i<=5;i++){//闪烁5次
	gpio_setOutput(GPIO0,  0xFF);
	bsp_uDelay(LOOP_UDELAY*10);
	gpio_setOutput(GPIO0,  0x00);
	bsp_uDelay(LOOP_UDELAY*10);
	}
        for (int i = 0; i < 8; i++) {
            gpio_setOutput(GPIO0, (1 << i) & 0xFF);    // 设置GPIO0输出为该模式
            bsp_uDelay(LOOP_UDELAY*10);               // 延迟一段时间
        }

        for (int i = 7; i >= 0; i--) { // 从7开始递减到0，共8个状态，实现倒着的效果

            gpio_setOutput(GPIO0, (1 << i) & 0xFF);    // 设置GPIO0输出为该模式
            bsp_uDelay(LOOP_UDELAY * 10);               // 延迟一段时间
        }


        bsp_printf("new led ! \r\n");

        gpio_setOutput(GPIO0, 0x00);
        bsp_uDelay(LOOP_UDELAY * 40);
        // 先点亮中间两个灯 4 5
        gpio_setOutput(GPIO0, (1 << 3) | (1 << 4));
        bsp_uDelay(LOOP_UDELAY * 30);


        int side_to_mid_count = 3;
            for (int j = 0; j < side_to_mid_count; j++) {
                // 从两边往中间移动左边部分
                for (int i = 0 + j; i <= 3; i++) {

                    int current_output = (1 << i) | (1 << (7 - i));
                    gpio_setOutput(GPIO0, current_output);
                    bsp_uDelay(LOOP_UDELAY * 30);

                      // 从两边往中间移动右边部分
                for (int i = 7 - j; i >= 4; i--) {

                    int current_output = (1 << i) | (1 << (7 - i));
                    gpio_setOutput(GPIO0, current_output);
                    bsp_uDelay(LOOP_UDELAY * 30);
                }

                    }
                }
            uint8_t value = 0;  // 初始化为0（对应全灭状态）
                    for (int k = 0; k < 255; k++) {
                        gpio_setOutput(GPIO0,value);
                        bsp_uDelay(LOOP_UDELAY * 5);
                        value++;  //每次递增1
                        if (value > 255) {  // 超过0xFF后重置为0，重新开始循环
                            value = 0;
                        }
                    }
}

}





//void main() {
//bsp_init();
//bsp_printf("gpio 0 demo ! \r\n");
//bsp_printf("onboard LEDs blinking \r\n");
////configure 4 bits gpio 0
////    gpio_setOutputEnable(GPIO0, 0xe);
////    gpio_setOutput(GPIO0, 0x0);
//
////修改的代码模块
//gpio_setOutputEnable(GPIO0, 0xFF); // 0x0F = 11111111，启用GPIO0的低4位
//    gpio_setOutput(GPIO0, 0x00);       // 初始状态全部LED熄灭
//
//    // 流水灯效果
//    for (int i = 0; i < 8; i++) { // 16个状态，包括全灭（可选）
//         // 生成一个只在第i位为1，其他位为0的模式
//        gpio_setOutput(GPIO0, (1 << i) & 0xFF);    // 设置GPIO0输出为该模式
//        bsp_uDelay(LOOP_UDELAY);               // 延迟一段时间
//    }
//    while (1) {
//            for (int i = 0; i < 8; i++) {
//
//                gpio_setOutput(GPIO0, (1 << i) & 0xFF);
//                bsp_uDelay(LOOP_UDELAY);
//
//            }
//        }
//    init();



    //下面四行代码为原本demo示例
//    for (int i=0; i<50; i=i+1) {
//        gpio_setOutput(GPIO0, gpio_getOutput(GPIO0) ^ 0xe);
//        bsp_uDelay(LOOP_UDELAY);
//    }
//    bsp_printf("gpio 0 interrupt demo ! \r\n");
//    bsp_printf("Ti180 press and release onboard button sw4 \r\n");
//    bsp_printf("Ti60 press and release onboard button sw6 \r\n");
//    bsp_printf("T120 press and release onboard button sw7 \r\n");
//    init();
//    while(1);
//}
#else
/******************************************************************************
*
* @brief This main function is executed when GPIO functionality is disabled.
*        It initializes the BSP and prints a message indicating that
*        GPIO 0 is disabled, and the user should enable it to run the app.
*
******************************************************************************/
void main() {
    bsp_init();
    bsp_printf("gpio 0 is disabled, please enable it to run this app.\r\n");
}
#endif

