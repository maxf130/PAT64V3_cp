#include "analysis_utils.h"

int OnCountPixels(const cv::Mat img, unsigned int pottop, unsigned int planttop) {
  cv::Mat result;

  if(img.channels()!=1)
    cvtColor(img, result, CV_BGR2GRAY);
  else
    result=img.clone();

  int i, j;
  int count=0;

  for(j=planttop; j<pottop; j++)
    for(i=result.cols*0.15; i<result.cols*0.85; i++)
    {
      if(*(result.data+j*result.step+i)<=210)
      {
        count=count+1;
      }
    }
  return count;
}

cv::Mat RestoreImgFromTemp(const cv::Mat temp, const cv::Mat source) {
  //temp is the template holding the information where leaf pixels are. source is the original input image
  auto output=source.clone();

  int i, j;
  for(i=0; i<temp.cols; i++)
    for(j=0; j<temp.rows; j++)
    {
      if(*(temp.data+j*temp.step+i)!=0)
      {
        *(output.data+i*source.channels()+j*source.step)=*(source.data+i*source.channels()+j*source.step);
        *(output.data+i*source.channels()+j*source.step+1)=*(source.data+i*source.channels()+j*source.step+1);
        *(output.data+i*source.channels()+j*source.step+2)=*(source.data+i*source.channels()+j*source.step+2);
      }
      else
      {
        *(output.data+i*source.channels()+j*source.step)=255;
        *(output.data+i*source.channels()+j*source.step+1)=255;
        *(output.data+i*source.channels()+j*source.step+2)=255;
      }
    }

  return output;
}

std::array<cv::Point, 3> FindLTPotLimits(const cv::Mat img) {
  std::array<cv::Point, 3> pots; 
  cv::Mat result;

  if(img.channels()!=1)
    cvtColor(img, result, CV_BGR2GRAY);
  else
    result=img.clone();

  int i, j;
  int count=0;

  int y=10000;
  int maxx=0;
  int minx=10000;
  int miny=0;
  int maxy=0;

  /* This for loop sets the variable y to the value of the lowest row
   * where the intensity changes by more than 10 from one row to the
   * next for the first time in that column.
   *
   * In this image '.' represents intensity 0, '#' represents intensity
   * 20.  Rows are labeled:
   *
   * 0: .....
   * 1: ...#.
   * 2: .###.
   * 3: ####.
   * 4: #####
   * 5: #####
   * 6: #####
   *
   * In this case the variable y is set to 4.  The lowest row in which any 
   * column changes intensity by more than 10.
   *
   * y seems to represent a boundary on where pot pixels are likely to be found
   */
  // In the central 20% of columns in steps of 5
  for(i=result.cols*0.40; i<result.cols*0.6; i=i+5)
    // In the bottom half of the image - 20
    for(j=result.rows*0.5; j<result.rows-20; j++)
    {
      // Difference between This pixel and  the same pixel 
      // in the previous row > 10
      if(abs(*(result.data+j*result.step+i)-*(result.data+(j-1)*result.step+i))>10)
      {

        if(j<y) {
          y=j;
        }

        break;
      }
    }

  /* 
   * The point pots[0] is a point in the center column of the image, at the
   * position of the variable y (see above) with some buffer if possible
   */
  pots[0].x=result.cols/2;
  if(y+10<result.rows)
    pots[0].y=y+10;
  else
    pots[0].y=y;

#pragma omp parallel for
  /*
   * Sets minx to the left most column where there is a sudden change in
   * intensity, for any row below y (see above).  miny is set to the 
   * corresponding row.
   *
   * maxx/maxy are analogous
   */
  // For every row in the image from y to nearly the bottom
  for(j=pots[0].y; j<result.rows-20; j++)
    // For every central column (The edges are likely to contain fluff)
    for(i=result.cols*0.20; i<result.cols*0.8; i++)

    {
      // If Intensity changes from this column to the next 
      // (in the current row)
      if(abs(*(result.data+j*result.step+i)-*(result.data+j*result.step+i+1))>10)
      {
        if(i<minx)
        {
          minx=i+1;
          miny=j;
        }
        if(i>maxx)
        {
          maxx=i-1;
          maxy=j;
        }
      }
    }

  pots[1].x=minx;
  pots[1].y=miny;

  pots[2].x=maxx;
  pots[2].y=maxy;


  // Returning std::array by value.  Should use RVO.
  return pots;
}



