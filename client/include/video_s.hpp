#ifndef _VIDEO_S_H
#define _VIDEO_S_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

using namespace cv;

typedef enum
{
  VID_NO_INIT = 0,
  VID_INIT,
  VID_READY,
}stateVideoS;

class video_streaming_c {
  public:
  video_streaming_c(size_t, size_t);
  video_streaming_c(char *, char *, size_t, size_t);
  video_streaming_c(void);
  void video_init_buffer(size_t, size_t);
  ~video_streaming_c(void);
  int show_image(void);
  void wait_end(void);
  char *video_buff[2];
  int size_video_buff;
  int state;
  Mat img[2];
  size_t number_img;
  private:
  int wait_show(void);
  void img_to_screen(void);
  pthread_t video_s;
  pthread_attr_t	attr;
  size_t size;
  size_t width, height;
};

void video_receive_thd(void * pv);


#endif