import time
import os
import sys
import math
import ujson  # 添加ujson库用于JSON格式的读写

from media.sensor import *
from media.display import *
from media.media import *
from time import ticks_ms
from machine import FPIOA
from machine import Pin
from machine import TOUCH
from machine import UART

sensor = None

# 计算两点之间的距离
def distance(p1, p2):
    return math.sqrt((p1[0]-p2[0])**2 + (p1[1]-p2[1])**2)

# 计算两条线之间的角度（度）
def angle_between_lines(p1, p2, p3):
    # 向量1: p2 -> p1
    v1x = p1[0] - p2[0]
    v1y = p1[1] - p2[1]
    # 向量2: p2 -> p3
    v2x = p3[0] - p2[0]
    v2y = p3[1] - p2[1]

    # 点积
    dot_product = v1x*v2x + v1y*v2y
    # 模长
    mag1 = math.sqrt(v1x**2 + v1y**2)
    mag2 = math.sqrt(v2x**2 + v2y**2)

    if mag1 == 0 or mag2 == 0:
        return 0

    # 计算角度（弧度转度）
    angle = math.acos(max(-1, min(1, dot_product/(mag1*mag2)))) * 180 / math.pi
    return min(angle, 180 - angle)  # 取最小角度

# 绘制十字标记
def draw_crosshair(img, x, y, size=5, color=(255,0,0), thickness=2):
    # 水平线段
    img.draw_line(x - size, y, x + size, y, color, thickness)
    # 垂直线段
    img.draw_line(x, y - size, x, y + size, color, thickness)

# 从文件加载阈值
def load_thresholds():
    try:
        # 尝试打开并读取阈值文件
        f = open('/sdcard/thresholds.json', 'r')  # 使用SD卡路径
        data = ujson.load(f)
        f.close()
        print("从文件加载阈值:", data)
        return data
    except Exception as e:
        print("加载阈值失败:", e)
        # 如果文件不存在或读取失败，返回默认值
        return {
            'rect': [[0, 62]]  # 矩形识别的阈值
        }

# 保存阈值到文件
def save_thresholds(thresholds):
    try:
        # 尝试写入阈值文件
        f = open('/sdcard/thresholds.json', 'w')  # 使用SD卡路径
        ujson.dump(thresholds, f)
        f.close()
        print("阈值保存成功")
        return True
    except Exception as e:
        print("保存阈值失败:", e)
        return False
try:
    # 初始化传感器
    print("初始化传感器...")
    sensor = Sensor(width=320, height=240)
    sensor.reset()
    # 设置帧大小为800x480以提高处理速度
    sensor.set_framesize(width=320, height=240)
    sensor.set_pixformat(Sensor.RGB565)

    # 初始化显示
    print("初始化显示...")
    Display.init(Display.ST7701, to_ide=True, width=800, height=480)
    MediaManager.init()
    sensor.run()
    print("传感器启动成功")

    # 计算显示偏移量
    disp_x = round((800 - sensor.width()) / 2)
    disp_y = round((480 - sensor.height()) / 2)

    fpioa = FPIOA()

    # 初始化串口
    print("初始化串口...")
    fpioa.set_function(3, FPIOA.UART1_TXD)
    fpioa.set_function(4, FPIOA.UART1_RXD)
    uart = UART(UART.UART1, 9600)
    print("串口初始化完成")

    # 初始化触摸屏
    print("初始化触摸屏...")
    tp = TOUCH(0)
    print("触摸屏初始化完成")

    clock = time.clock()
    flag = 0  # 状态标识: 0-待机, 2-阈值调整模式
    # 新增：用于跟踪未检测到矩形的帧数和上一次有效数据
    missed_frames = 0  # 连续未检测到矩形的帧数
    last_dx = None     # 上一次检测到的dx
    last_dy = None     # 上一次检测到的dy

    # 从文件加载阈值或使用默认值
    threshold_dict = load_thresholds()
    print("当前阈值:", threshold_dict)

    # 设置固定点（屏幕中心上方一点）
    FIXED_POINT_X = sensor.width() // 2  # 图像中心X坐标
    FIXED_POINT_Y = sensor.height() // 2 - 20  # 图像中心上方20像素

    # 使用时间戳的长按检测
    touch_start_time = 0
    TOUCH_LONG_PRESS_MS = 1000  # 1秒长按

    while True:
        clock.tick()
        os.exitpoint()

        # 拍摄图像
        img = sensor.snapshot(chn=CAM_CHN_ID_0)

        # 绘制固定点（蓝色十字）
        draw_crosshair(img, FIXED_POINT_X, FIXED_POINT_Y, size=8, color=(0,0,255), thickness=2)

        # 矩形识别部分
        rect_detected = False

        # 图像处理
        img_rect = img.to_grayscale(copy=True)
        img_rect = img_rect.binary(threshold_dict['rect'])
        rects = img_rect.find_rects(threshold=5000)

        for rect in rects:
            # 获取矩形四个角点
            corners = rect.corners()

            # 1. 计算矩形面积和周长，过滤过小或过大的矩形
            # 计算四条边的长度
            side1 = distance(corners[0], corners[1])
            side2 = distance(corners[1], corners[2])
            side3 = distance(corners[2], corners[3])
            side4 = distance(corners[3], corners[0])

            # 计算面积（近似）
            area = side1 * side2
            # 过滤面积：太小或太大的都过滤掉
            if area < 500 or area > 320*240*0.8:  # 根据实际分辨率调整
                continue

            # 2. 过滤宽高比不合理的矩形（接近正方形或特定比例）
            # 计算宽高比（长边/短边）
            if side1 > 0 and side2 > 0:
                aspect_ratio = max(side1/side2, side2/side1)
                # 过滤宽高比过大的（不是矩形的）
                if aspect_ratio > 5:  # 可根据实际情况调整
                    continue

            # 3. 过滤角度不接近直角的四边形（矩形四个角应接近90度）
            angles = [
                angle_between_lines(corners[3], corners[0], corners[1]),
                angle_between_lines(corners[0], corners[1], corners[2]),
                angle_between_lines(corners[1], corners[2], corners[3]),
                angle_between_lines(corners[2], corners[3], corners[0])
            ]

            # 检查所有角是否接近接近直角（80-100度之间）
            valid_angles = all(70 < angle < 110 for angle in angles)
            if not valid_angles:
                continue

            # 4. 检查对边是否近似相等（矩形对边相等）
            if abs(side1 - side3) > max(side1, side3)*0.3 or abs(side2 - side4) > max(side2, side4)*0.3:
                continue  # 对边差异超过20%则过滤

            # 经过所有过滤条件后，才认为是有效的矩形
            rect_detected = True  # 标记检测到有效矩形
            # 绘制矩形
            img.draw_line(corners[0][0], corners[0][1], corners[1][0], corners[1][1], color=(0,255,0), thickness=5)
            img.draw_line(corners[1][0], corners[1][1], corners[2][0], corners[2][1], color=(0,255,0), thickness=5)
            img.draw_line(corners[2][0], corners[2][1], corners[3][0], corners[3][1], color=(0,255,0), thickness=5)
            img.draw_line(corners[3][0], corners[3][1], corners[0][0], corners[0][1], color=(0,255,0), thickness=5)

            # 计算中心点
            center_x = int((corners[0][0] + corners[1][0] + corners[2][0] + corners[3][0]) / 4)
            center_y = int((corners[0][1] + corners[1][1] + corners[2][1] + corners[3][1]) / 4)

            # 绘制十字形中心点标记
            draw_crosshair(img, center_x, center_y, size=8, color=(255,0,0), thickness=2)

            # 计算固定点与中心点的距离
            dx = center_x - FIXED_POINT_X
            dy = FIXED_POINT_Y - center_y

            # 在图像上显示中心点坐标
            img.draw_string_advanced(5, 5, 20, f"FPS: {clock.fps():.1f}", color=(255, 0, 0))
            img.draw_string_advanced(5, 30, 20, f"中心: ({center_x}, {center_y})", color=(0, 255, 0))
            img.draw_string_advanced(5, 55, 20, f"X距离: {dx}", color=(255, 255, 0))
            img.draw_string_advanced(5, 80, 20, f"Y距离: {dy}", color=(255, 255, 0))




        # 处理通信逻辑
        if rect_detected:
            # 检测到有效矩形，更新历史数据并重置计数器
            missed_frames = 0
            last_dx = dx
            last_dy = dy
            # 发送当前数据
            data_trans = f"AA,{last_dx},{last_dy},FF"
            uart.write(data_trans.encode())
            print(f"矩形中心点: ({dx}, {dy})")
            print(data_trans.encode())
        else:
            # 未检测到矩形，更新计数器
            missed_frames += 1
            # 判断是否超过3帧未检测到
            if missed_frames <= 3 and last_dx is not None and last_dy is not None:
                # 3帧内，发送上一次数据
                data_trans = f"AA,{last_dx},{last_dy},FF"
                uart.write(data_trans.encode())
                print(f"使用上一次数据: {data_trans.encode()}")
            else:
                # 超过3帧或从未检测到，发送#
                uart.write('#')
                print('#')



            # 显示状态信息
            img.draw_string_advanced(5, 5, 20, f"FPS: {clock.fps():.1f}", color=(255, 0, 0))
            img.draw_string_advanced(5, 30, 20, "未检测到矩形", color=(255, 0, 0))

        # 在屏幕上居中显示图像
        Display.show_image(img, x=disp_x, y=disp_y)

        # 长按屏幕检测（使用时间戳）
        points = tp.read()
        if points:
            if touch_start_time == 0:
                touch_start_time = ticks_ms()
                print("触摸开始")
            else:
                duration = ticks_ms() - touch_start_time
                if duration > TOUCH_LONG_PRESS_MS:
                    flag = 2
                    touch_start_time = 0
                    print("长按进入阈值调整模式")
        else:
            if touch_start_time != 0:
                print("触摸结束")
            touch_start_time = 0

        # 阈值调整模式
        if flag == 2:
            print("进入阈值调整模式...")
            # UI参数定义
            button_color = (150, 150, 150)
            text_color = (0, 0, 0)
            highlight_color = (200, 0, 0)

            # UI位置常量
            LEFT_BUTTON_X = 0
            RIGHT_BUTTON_X = 640
            TOP_BUTTON_Y = 0
            BOTTOM_BUTTON_Y = 440
            BUTTON_WIDTH = 160
            BUTTON_HEIGHT = 40
            SLIDER_Y_START = 60
            SLIDER_SPACING = 60

            threshold_mode = 'rect'  # 只有矩形模式
            threshold_current = list(threshold_dict['rect'][0])  # 使用当前阈值作为初始值

            last_button_press = 0  # 防止按钮连击

            while flag == 2:
                os.exitpoint()
                # 创建白色背景画布
                img = image.Image(800, 480, image.RGB565)
                img.draw_rectangle(0, 0, 800, 480, color=(255, 255, 255), thickness=2, fill=True)

                # 获取摄像头图像
                img_snap = sensor.snapshot(chn=CAM_CHN_ID_0)

                # 处理图像用于二值化显示
                img_gray = img_snap.to_grayscale()
                img_bin = img_gray.binary([threshold_current[:2]])
                img_bin_rgb = img_bin.to_rgb565()

                # 居中显示处理后的图像
                img_bin_x = disp_x
                img_bin_y = disp_y
                img.draw_image(img_bin_rgb, img_bin_x, img_bin_y)
                img.draw_rectangle(img_bin_x, img_bin_y, img_bin_rgb.width(), img_bin_rgb.height(),
                                   color=(0, 255, 0), thickness=2)

                # 左上按钮 (返回)
                img.draw_rectangle(LEFT_BUTTON_X, TOP_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                   color=button_color, thickness=2, fill=True)
                img.draw_string_advanced(LEFT_BUTTON_X + 70, TOP_BUTTON_Y + 10, 30, "返回", color=text_color)

                # 左下按钮 (重置)
                img.draw_rectangle(LEFT_BUTTON_X, BOTTOM_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                   color=button_color, thickness=2, fill=True)
                img.draw_string_advanced(LEFT_BUTTON_X + 70, BOTTOM_BUTTON_Y + 10, 30, "归位", color=text_color)

                # 右下按钮 (保存)
                img.draw_rectangle(RIGHT_BUTTON_X, BOTTOM_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                   color=button_color, thickness=2, fill=True)
                img.draw_string_advanced(RIGHT_BUTTON_X + 70, BOTTOM_BUTTON_Y + 10, 30, "保存", color=text_color)

                # 滑块区域
                for j in [LEFT_BUTTON_X, RIGHT_BUTTON_X]:
                    for i in range(0, 2):  # 只有两个滑块
                        y_pos = SLIDER_Y_START + i * SLIDER_SPACING
                        img.draw_rectangle(j, y_pos, BUTTON_WIDTH, BUTTON_HEIGHT,
                                           color=button_color, thickness=2, fill=True)

                        # 左侧滑块标记"-", 右侧滑块标记"+"
                        marker = "-" if j == LEFT_BUTTON_X else "+"
                        img.draw_string_advanced(j + 70, y_pos + 10, 30, marker, color=text_color)

                # 显示当前模式
                mode_text = f"调整:矩形阈值"
                img.draw_string_advanced(310, 15, 25, mode_text, color=(0, 0, 0))

                # 显示滑块值
                for i in range(2):
                    y_pos = SLIDER_Y_START + i * SLIDER_SPACING
                    real_value = threshold_current[i] - 127
                    val_text = f"阈值{i}:{threshold_current[i]} (实际:{real_value})"
                    img.draw_string_advanced(200, y_pos + 10, 20, val_text, color=(0, 0, 255))

                # 全屏显示
                Display.show_image(img)

                # 处理触摸输入
                points = tp.read()
                if points:
                    # 坐标映射函数
                    def witch_key(x, y):
                        # 检查左侧按钮列
                        if LEFT_BUTTON_X <= x < LEFT_BUTTON_X + BUTTON_WIDTH:
                            # 顶部按钮
                            if TOP_BUTTON_Y <= y < TOP_BUTTON_Y + BUTTON_HEIGHT:
                                return "return"
                            # 底部按钮
                            elif BOTTOM_BUTTON_Y <= y < BOTTOM_BUTTON_Y + BUTTON_HEIGHT:
                                return "reset"
                            # 滑块按钮
                            for i in range(2):
                                y_pos = SLIDER_Y_START + i * SLIDER_SPACING
                                if y_pos <= y < y_pos + BUTTON_HEIGHT:
                                    return str(i)  # 0或1

                        # 检查右侧按钮列
                        elif RIGHT_BUTTON_X <= x < RIGHT_BUTTON_X + BUTTON_WIDTH:
                            # 底部按钮
                            if BOTTOM_BUTTON_Y <= y < BOTTOM_BUTTON_Y + BUTTON_HEIGHT:
                                return "save"
                            # 滑块按钮
                            for i in range(2):
                                y_pos = SLIDER_Y_START + i * SLIDER_SPACING
                                if y_pos <= y < y_pos + BUTTON_HEIGHT:
                                    return str(i + 2)  # 2或3
                        return None

                    button_ = witch_key(points[0].x, points[0].y)
                    current_time = ticks_ms()

                    # 防连击处理
                    if button_ and (current_time - last_button_press > 300):
                        last_button_press = current_time
                        print(f"按钮按下: {button_}")

                        # 返回
                        if button_ == "return":
                            flag = 0
                            print("退出阈值调整模式")
                            time.sleep_ms(500)
                            break
                        # 重置滑块
                        elif button_ == "reset":
                            threshold_current = [0, 62]
                            print("滑块恢复默认")
                            # 显示恢复成功
                            img.draw_rectangle(200, 200, 300, 40, color=button_color, thickness=2, fill=True)
                            img.draw_string_advanced(200, 200, 30, "滑块恢复默认", color=text_color)
                            Display.show_image(img)
                            time.sleep_ms(1000)
                        # 保存阈值
                        elif button_ == "save":
                            new_threshold = threshold_current[:2]
                            threshold_dict['rect'] = [new_threshold]
                            print(f"保存矩形阈值: {new_threshold}")

                            # 保存到文件
                            if save_thresholds(threshold_dict):
                                # 显示保存成功
                                img.draw_rectangle(200, 200, 300, 40, color=button_color, thickness=2, fill=True)
                                img.draw_string_advanced(200, 200, 30, "保存成功", color=text_color)
                            else:
                                # 显示保存失败
                                img.draw_rectangle(200, 200, 300, 40, color=button_color, thickness=2, fill=True)
                                img.draw_string_advanced(200, 200, 30, "保存失败", color=(255,0,0))
                            Display.show_image(img)
                            time.sleep_ms(1000)
                        # 调整滑块
                        else:
                            idx = int(button_)
                            if idx >= 2:  # 右侧按钮（增加）
                                idx -= 2
                                threshold_current[idx] = min(255, threshold_current[idx] + 5)
                                print(f"增加滑块 {idx}: {threshold_current[idx]}")
                            else:  # 左侧按钮（减少）
                                threshold_current[idx] = max(0, threshold_current[idx] - 5)
                                print(f"减少滑块 {idx}: {threshold_current[idx]}")

except KeyboardInterrupt as e:
    print("用户停止: ", e)
except Exception as e:
    print(f"异常: {e}")
    import sys
    try:
        sys.print_exception(e)
    except AttributeError:
        pass
finally:
    print("清理资源...")
    if isinstance(sensor, Sensor):
        sensor.stop()
    Display.deinit()
    os.exitpoint(os.EXITPOINT_ENABLE_SLEEP)
    time.sleep_ms(100)
    MediaManager.deinit()
    print("程序结束")
