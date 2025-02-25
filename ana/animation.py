import cv2
import os
import re

image_folder = "ana2/figure/cs"
video_file = "ana2/figure/cs/cs_animation.mp4"

# フレームレートを設定
fps = 3

# 正規表現のパターンを指定 (例: "image_001.png", "image_002.png", ...)
pattern = r"^cross_section_(\d+)\.png$"


# 特定のパターンに一致する画像ファイルのリストを取得し、数値順にソート
def sort_key(filename):
    match = re.match(pattern, filename)
    if match:
        return int(match.group(1))
    else:
        return float("inf")


images = [img for img in os.listdir(image_folder) if re.match(pattern, img)]
images.sort(key=sort_key)


# 最初の画像を読み込み、サイズを取得
if not images:
    print("cannot find images")
else:
    frame = cv2.imread(os.path.join(image_folder, images[0]))
    height, width, layers = frame.shape

    # 動画の書き出し設定
    fourcc = cv2.VideoWriter_fourcc(*"mp4v")  # 'mp4v'でmp4形式の動画を出力
    video = cv2.VideoWriter(video_file, fourcc, fps, (width, height))

    # 画像を順番に読み込み、動画に追加
    for image in images:
        print("processing:", image)
        img_path = os.path.join(image_folder, image)
        frame = cv2.imread(img_path)
        video.write(frame)

    # 動画の書き出しを終了
    video.release()
    cv2.destroyAllWindows()

    print("created video:", video_file)
