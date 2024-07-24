// -*- C++ -*-
// <rtc-template block="description">
/*!
 * @file  RealSense_data.cpp
 * @brief data from RealSense
 *
 */
// </rtc-template>

#include "RealSense_data.h"

// フレームサイズ（RealSense D415のデフォルト）
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

// Module specification
// <rtc-template block="module_spec">
#if RTM_MAJOR_VERSION >= 2
static const char* const realsense_data_spec[] =
#else
static const char* realsense_data_spec[] =
#endif
  {
    "implementation_id", "RealSense_data",
    "type_name",         "RealSense_data",
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
RealSense_data::RealSense_data(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_marker_traceOut("marker_trace", m_marker_trace),
    m_horizontal_scanOut("horizontal_scan", m_horizontal_scan)
    // </rtc-template>
{
}

/*!
 * @brief destructor
 */
RealSense_data::~RealSense_data()
{
}



RTC::ReturnCode_t RealSense_data::onInitialize()
{
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  
  // Set OutPort buffer
  addOutPort("marker_trace", m_marker_traceOut);
  addOutPort("horizontal_scan", m_horizontal_scanOut);

  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
  // </rtc-template>

  // <rtc-template block="bind_config">
  // </rtc-template>

  
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t RealSense_data::onFinalize()
{
  return RTC::RTC_OK;
}
*/


//RTC::ReturnCode_t RealSense_data::onStartup(RTC::UniqueId /*ec_id*/)
//{
//  return RTC::RTC_OK;
//}


//RTC::ReturnCode_t RealSense_data::onShutdown(RTC::UniqueId /*ec_id*/)
//{
//  return RTC::RTC_OK;
//}


RTC::ReturnCode_t RealSense_data::onActivated(RTC::UniqueId /*ec_id*/)
{
    m_marker_trace.data.length(5);
    m_horizontal_scan.data.length(FRAME_WIDTH);
  return RTC::RTC_OK;
}


RTC::ReturnCode_t RealSense_data::onDeactivated(RTC::UniqueId /*ec_id*/)
{
  return RTC::RTC_OK;
}


RTC::ReturnCode_t RealSense_data::onExecute(RTC::UniqueId /*ec_id*/)
{
    bool write_this_time = false;

    try {
        // RealSenseパイプラインの初期化
        rs2::pipeline pipe;
        rs2::config cfg;
        cfg.enable_stream(RS2_STREAM_COLOR, FRAME_WIDTH, FRAME_HEIGHT, RS2_FORMAT_BGR8, 30);
        cfg.enable_stream(RS2_STREAM_DEPTH, FRAME_WIDTH, FRAME_HEIGHT, RS2_FORMAT_Z16, 30);

        // フレーム同期用のsyncerを作成
        rs2::syncer sync;
        pipe.start(cfg, sync);

        while (cv::waitKey(1) < 0) {
            // フレームを取得
            rs2::frameset frames = sync.wait_for_frames();
            rs2::video_frame color_frame = frames.get_color_frame();
            rs2::depth_frame depth_frame = frames.get_depth_frame().as<rs2::depth_frame>();

            // OpenCV形式に変換
            cv::Mat color(FRAME_HEIGHT, FRAME_WIDTH, CV_8UC3, (void*)color_frame.get_data());
            cv::Mat depth(FRAME_HEIGHT, FRAME_WIDTH, CV_16UC1, (void*)depth_frame.get_data());

            // 緑色の画素を検出
            cv::Mat red_mask;
            //Blue, Green, Redの順
            cv::inRange(color, cv::Scalar(0, 100, 0), cv::Scalar(80, 255, 80), red_mask);

            // 輪郭を検出
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(red_mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            if (contours.empty()) {
                //std::cout << "No Green cluster found." << std::endl;
                m_marker_trace.data[0] = 0;
                m_marker_trace.data[1] = 0;
                m_marker_trace.data[2] = 0;
                m_marker_trace.data[3] = 0;
                m_marker_trace.data[4] = 0;

                m_marker_traceOut.write();
                write_this_time = true;
                continue;
            }

            // 最大の緑色クラスタを検出
            double max_area = 0;
            std::vector<cv::Point> largest_contour;
            for (const auto& contour : contours) {
                double area = cv::contourArea(contour);
                if (area > max_area) {
                    max_area = area;
                    largest_contour = contour;
                }
            }

            // 重心を計算
            cv::Moments m = cv::moments(largest_contour);
            int cx = static_cast<int>(m.m10 / m.m00);
            int cy = static_cast<int>(m.m01 / m.m00);

            // 重心の位置が画像範囲内であることを確認
            if (cx < 0 || cx >= FRAME_WIDTH || cy < 0 || cy >= FRAME_HEIGHT) {
                //std::cerr << "Centroid out of range: (" << cx << ", " << cy << ")" << std::endl;
                m_marker_trace.data[0] = 0;
                m_marker_trace.data[1] = 0;
                m_marker_trace.data[2] = 0;
                m_marker_trace.data[3] = 0;
                m_marker_trace.data[4] = 0;

                m_marker_traceOut.write();
                write_this_time = true;
                continue;
            }

            // 重心の深度データを取得
            float distance = depth_frame.get_distance(cx, cy);

            // 緑色の画素の深度データと角度を表示
            for (int y = 0; y < red_mask.rows; y++) {
                for (int x = 0; x < red_mask.cols; x++) {
                    if (red_mask.at<uchar>(y, x) > 0) {
                        float distance = depth_frame.get_distance(x, y);

                        // カメラの視野角 (仮定として、RealSense D415の視野角を使用)
                        const float hfov = 69.4f; // 水平方向視野角
                        const float vfov = 42.5f; // 垂直方向視野角

                        // ピクセル位置から角度を計算
                        float center_x = red_mask.cols / 2.0f;
                        float center_y = red_mask.rows / 2.0f;

                        float angle_x = (x - center_x) * (hfov / red_mask.cols);
                        float angle_y = (y - center_y) * (vfov / red_mask.rows);

                        /*std::cout << "Red pixel at (" << x << ", " << y << "): Depth = " << distance << " meters, "
                            << "Angle (X, Y) = (" << angle_x << "°, " << angle_y << "°)" << std::endl;*/
                    }
                }
            }

            // カメラの視野角 (仮定として、RealSense D415の視野角を使用)
            const float hfov = 69.4f; // 水平方向視野角
            const float vfov = 42.5f; // 垂直方向視野角

            // ピクセル位置から角度を計算
            float center_x = red_mask.cols / 2.0f;
            float center_y = red_mask.rows / 2.0f;


            float angle_x = (cx - center_x) * (hfov / red_mask.cols);
            float angle_y = (cy - center_y) * (vfov / red_mask.rows);

            //std::cout << red_mask.cols << ", " << red_mask.rows << std::endl;

            std::cout << "Green cluster centroid at (" << cx << ", " << cy << "): Depth = " << distance << " meters, "
                << "Angle (X, Y) = (" << angle_x << "°, " << angle_y << "°)" << std::endl;

            //マーカー追従データの出力
            m_marker_trace.data[0] = cx;
            m_marker_trace.data[1] = cy;
            m_marker_trace.data[2] = distance;
            m_marker_trace.data[3] = angle_x;
            m_marker_trace.data[4] = angle_y;

            m_marker_traceOut.write();
            write_this_time = true;

            //障害物スキャンデータの出力
            for (int i = 0; i < red_mask.cols; i++) {
                if (red_mask.at<uchar>(red_mask.rows - 1, i) > 0) {
                    m_horizontal_scan.data[i] = depth_frame.get_distance(i, red_mask.rows - 1);
                }
                else {
                    m_horizontal_scan.data[i] = 1000;
                }
            }

            // 画像を表示
            cv::circle(color, cv::Point(cx, cy), 5, cv::Scalar(0, 255, 0), -1);
            cv::imshow("Color Image", color);
            cv::imshow("Red Mask", red_mask);
        }

        pipe.stop();
    }
    catch (const rs2::error& e) {
        std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n"
            << e.what() << std::endl;
        return RTC::RTC_ERROR;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return  RTC::RTC_ERROR;
    }
    if (write_this_time == false) {
        m_marker_trace.data[0] = 0;
        m_marker_trace.data[1] = 0;
        m_marker_trace.data[2] = 0;
        m_marker_trace.data[3] = 0;
        m_marker_trace.data[4] = 0;

        m_marker_traceOut.write();
    }

    return RTC::RTC_OK;

}


//RTC::ReturnCode_t RealSense_data::onAborting(RTC::UniqueId /*ec_id*/)
//{
//  return RTC::RTC_OK;
//}


//RTC::ReturnCode_t RealSense_data::onError(RTC::UniqueId /*ec_id*/)
//{
//  return RTC::RTC_OK;
//}


//RTC::ReturnCode_t RealSense_data::onReset(RTC::UniqueId /*ec_id*/)
//{
//  return RTC::RTC_OK;
//}


//RTC::ReturnCode_t RealSense_data::onStateUpdate(RTC::UniqueId /*ec_id*/)
//{
//  return RTC::RTC_OK;
//}


//RTC::ReturnCode_t RealSense_data::onRateChanged(RTC::UniqueId /*ec_id*/)
//{
//  return RTC::RTC_OK;
//}



extern "C"
{
 
  void RealSense_dataInit(RTC::Manager* manager)
  {
    coil::Properties profile(realsense_data_spec);
    manager->registerFactory(profile,
                             RTC::Create<RealSense_data>,
                             RTC::Delete<RealSense_data>);
  }
  
}
