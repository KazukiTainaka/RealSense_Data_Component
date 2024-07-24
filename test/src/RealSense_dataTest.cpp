// -*- C++ -*-
// <rtc-template block="description">
/*!
 * @file  RealSense_dataTest.cpp
 * @brief data from RealSense (test code)
 *
 */
// </rtc-template>

#include "RealSense_dataTest.h"

// Module specification
// <rtc-template block="module_spec">
#if RTM_MAJOR_VERSION >= 2
static const char* const realsense_data_spec[] =
#else
static const char* realsense_data_spec[] =
#endif
  {
    "implementation_id", "RealSense_dataTest",
    "type_name",         "RealSense_dataTest",
    "description",       "data from RealSense",
    "version",           "1.0.0",
    "vendor",            "Tail Seekers",
    "category",          "Motion Capture",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    ""
  };
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
RealSense_dataTest::RealSense_dataTest(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_marker_traceIn("marker_trace", m_marker_trace),
    m_horizontal_scanIn("horizontal_scan", m_horizontal_scan)

    // </rtc-template>
{
}

/*!
 * @brief destructor
 */
RealSense_dataTest::~RealSense_dataTest()
{
}



RTC::ReturnCode_t RealSense_dataTest::onInitialize()
{
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  addInPort("marker_trace", m_marker_traceIn);
  addInPort("horizontal_scan", m_horizontal_scanIn);
  
  // Set OutPort buffer
  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
  // </rtc-template>

  // <rtc-template block="bind_config">
  // </rtc-template>
  
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t RealSense_dataTest::onFinalize()
{
  return RTC::RTC_OK;
}
*/


//RTC::ReturnCode_t RealSense_dataTest::onStartup(RTC::UniqueId /*ec_id*/)
//{
//  return RTC::RTC_OK;
//}


//RTC::ReturnCode_t RealSense_dataTest::onShutdown(RTC::UniqueId /*ec_id*/)
//{
//  return RTC::RTC_OK;
//}


RTC::ReturnCode_t RealSense_dataTest::onActivated(RTC::UniqueId /*ec_id*/)
{
  return RTC::RTC_OK;
}


RTC::ReturnCode_t RealSense_dataTest::onDeactivated(RTC::UniqueId /*ec_id*/)
{
  return RTC::RTC_OK;
}


RTC::ReturnCode_t RealSense_dataTest::onExecute(RTC::UniqueId /*ec_id*/)
{
  return RTC::RTC_OK;
}


//RTC::ReturnCode_t RealSense_dataTest::onAborting(RTC::UniqueId /*ec_id*/)
//{
//  return RTC::RTC_OK;
//}


//RTC::ReturnCode_t RealSense_dataTest::onError(RTC::UniqueId /*ec_id*/)
//{
//  return RTC::RTC_OK;
//}


//RTC::ReturnCode_t RealSense_dataTest::onReset(RTC::UniqueId /*ec_id*/)
//{
//  return RTC::RTC_OK;
//}


//RTC::ReturnCode_t RealSense_dataTest::onStateUpdate(RTC::UniqueId /*ec_id*/)
//{
//  return RTC::RTC_OK;
//}


//RTC::ReturnCode_t RealSense_dataTest::onRateChanged(RTC::UniqueId /*ec_id*/)
//{
//  return RTC::RTC_OK;
//}


bool RealSense_dataTest::runTest()
{
    return true;
}


extern "C"
{
 
  void RealSense_dataTestInit(RTC::Manager* manager)
  {
    coil::Properties profile(realsense_data_spec);
    manager->registerFactory(profile,
                             RTC::Create<RealSense_dataTest>,
                             RTC::Delete<RealSense_dataTest>);
  }
  
}
