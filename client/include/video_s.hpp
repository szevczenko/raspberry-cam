#ifndef _VIDEO_S_H
#define _VIDEO_S_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

using namespace cv;

class video_streaming_c {
  public:
  video_streaming_c(size_t, size_t);
  video_streaming_c(char *, size_t, size_t);
  ~video_streaming_c(void);
  int show_image(void);
  void wait_end(void);
  char *video_buff;
  Mat img;
  private:
  int wait_show(void);
  void img_to_screen(void);
  pthread_t video_s;
  pthread_attr_t	attr;
  size_t size;
  size_t width, height;
};

  void * video_receive_thd(void * pv);


#endif