//--------------------------------------------------------------------
//     1，2两个方向灯状态：  R1   R2   G1   G2   Y1   Y2      time
//ligth1的显示对应开发板上：led1 led2 led3 led4 led5 led6
//                   S0： 1     0    0     1    0    0       30s
//                   S1： 1     0    0     0    0    1       3s
//                   S2:  0     1    1     0    0    0       30s
//                   S3:  0     1    0     0    1    0       3s
module traffic_led(
input wire clk_50m,//输入时钟频率：1HZ/秒
input wire rst1,//rst1分别对应
input wire rst2,
input wire rst3,
input wire rst4,
 
output reg [5:0] light1,//显示六个交通灯
output   [1:0] seg_sel,        // 数码管位选，最左侧数码管为最高位
output   [6:0] seg_led         // 数码管段选
);
 
reg [1:0] state;//有限状态
reg [6:0] num;//7位寄存器 计数方便判断转移关系


reg [6:0] time_d;//统计不同状态的时间，便于数码管显示
reg [3:0] time_gewei; //拿出时间的个位十位
reg [3:0] time_shiwei;
wire [7:0] data_out;			   //要传递到数码管显示的数据

//时钟分频为1hz 即clk_1hz=1hz
reg [31:0] count;
reg clk_1hz;
always@(posedge clk_50m or negedge rst1 or negedge rst2 or  negedge rst3 or negedge rst4)
begin
    if(!rst1||!rst2|| !rst3||!rst3||!rst4)
    begin
    count<=32'd0;
    clk_1hz <=1'b0;
    end
    
    else if(count==32'd24999999)
    begin
    count<=32'd0;
    clk_1hz <= 1'b1;
    end 
    
    else 
       begin
    count<=count+1'b1;
    clk_1hz <= 1'b0;
        end
end


//定义有限状态
parameter S0 =3'b00;
parameter S1 =3'b01;
parameter S2 =3'b10;
parameter S3 =3'b11;
 
//7位计数器实现定时器功能
always@(posedge clk_1hz or negedge rst1 or negedge rst2 or  negedge rst3 or negedge rst4)
begin
  if(!rst1) begin num<=7'd0;end
  else if(!rst2) begin num<=7'd29;end
  else if(!rst3) begin num<=7'd32;end
  else if(!rst4) begin num<=7'd62;end
  else if(num==7'd65)
    begin
	 num<=7'd0;
	end
  else
	 begin
     num<=num+1;
	 end
end
 
//状态机模块，绿灯30秒，黄灯3秒
always@(posedge clk_1hz or negedge rst1 or negedge rst2 or  negedge rst3 or negedge rst4)
begin
if(!rst1) begin//检测到复位信号rst1后回到状态S0
	 state<=S0;
	 light1=6'b100_100; 
	end
else if(!rst2)begin //检测到复位信号rst2后回到状态S1
	state<=S1;
	light1=6'b100_001;
	end
else if(!rst3)begin //检测到复位信号rst3后回到状态S2
	state<=S2;
	light1=6'b011_000;
	end
else if(!rst4) begin //检测到复位信号rst4后回到状态S3
	 state<=S3;
	 light1=6'b010_010;
	end 
	
else 
    begin
	case(state)
     S0:
		 begin
			begin light1=6'b100_100;end //R1 G2亮 30s
			
		  if(num==7'd24||num==7'd26||num==7'd28)//G2绿灯的最后6秒实现闪烁
			  light1=6'b100_000;
			  
		  else if(num==7'd29)//从0开始计数计数到29即30秒时转移到状态S1
			  state<=S1;
		 end
	  S1:
		  begin
			begin light1=6'b100_001;end//R1 Y2亮 3s
			
			 if(num==7'd32)//Y2黄灯亮三秒后转移到状态S2
			 state<=S2;
		  end 
	  S2:
		  begin
			begin light1=6'b011_000;end //R2 G1 亮 30s
			
			 if(num==7'd57||num==7'd59||num==7'd61)//G1绿灯的最后6秒实现 闪烁
			  light1=6'b010_000;
			  
			 else if(num==7'd62)//计数到62即63秒后转移到状态S3
			  state<=S3;
			end 
		S3:
			begin
			 begin light1=6'b010_010;end //R2 Y1亮 3s
			  
			  if(num==7'd65)//三秒后转移到状态S0
			  state<=S0;
			end
		endcase
	end
end


//数码管显示模块 对num进行显示

always @(posedge clk_1hz) 
begin   //实现倒计时，num统计不同状态的时间，对其倒计时
 if(num==7'd0||num==7'd65)//S0状态
  time_d<=7'd30;
  else if(num==7'd29)//S1状态
  time_d<=7'd3;
  else if(num==7'd32)//S2状态
  time_d<=7'd30;
  else if(num==7'd62)//S3状态
  time_d<=7'd3;
  else time_d<=time_d-1'd1;
 begin
  time_gewei = time_d % 10;
  time_shiwei = time_d / 10;end
end
assign data_out={time_gewei,time_shiwei};


//例化数码管模块
seg_time seg_time
(
 .clk_50m     	(clk_50m),	       
 .rst1    		(rst1),	
 .rst2   		(rst2),
 .rst3    		(rst3),
 .rst4   		(rst4),
 .data    		(data_out),	      
 .seg_sel   	(seg_sel),	     
 .seg_led  		(seg_led)      
    );
	
	
	
endmodule
