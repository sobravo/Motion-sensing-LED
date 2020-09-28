# Motion-sensing-LED

	据统计，交通事故是造成4至14岁儿童非故意伤害的主要原因，是“头号杀手”。特别是对于小学生阶段的儿童，天性活泼好动，在上下学的路上往往控制不住自己，在路边奔跑跳跃的时候往往会导致交通意外事故。
	为了缓解这种问题，宋语新萌生了一个想法，就是在每天背着的书包上增加一个行为监测装置，如果发现步行的过程中有过于激烈的动作，自动发出警示：触发书包外部的LED灯带发出闪烁光，同时发出嗡鸣声，提醒周边的车辆和行人。
	主要实现原理：三轴加速度传感器进行行为姿态的检测，Arduino作为控制器接受检测信号，判断过激动作，并控制LED灯带发出闪烁光，蜂鸣器发声。Arduino的控制逻辑通过Scratch实现
