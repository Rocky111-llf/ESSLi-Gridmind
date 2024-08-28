/**
  ******************************************************************************
  * @file    RS485.c
  * @author  RZ
  * @brief   RS485.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 GridMind Co. Ltd.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "Includings.h"

S_RS485_WR* hRS485_WR = (S_RS485_WR*)0x42000000;					//ª˘µÿ÷∑…Ë÷√
S_RS485_RD* hRS485_RD = (S_RS485_RD*)((u32*)0x42000000+512);
