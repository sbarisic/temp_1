//-----------------------------------------------------------------------------
//
//    ** DO NOT EDIT THIS FILE! **
//    This file was generated by a tool
//    re-running the tool will overwrite this file.
//
//-----------------------------------------------------------------------------

#include "ESP32CAN.h"
#include "ESP32CAN_ESP32CAN_CAN.h"

using namespace ESP32CAN::ESP32CAN;


HRESULT Library_ESP32CAN_ESP32CAN_CAN::InitNative___STATIC__VOID( CLR_RT_StackFrame& stack )
{
    NANOCLR_HEADER(); hr = S_OK;
    {

        CAN::InitNative(  hr );
        NANOCLR_CHECK_HRESULT( hr );

    }
    NANOCLR_NOCLEANUP();
}