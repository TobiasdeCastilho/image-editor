#pragma once

#include <map>
#include <opencv4/opencv2/opencv.hpp>

#include "../utils/image.cpp"

int work_width = 720;
int work_height = 480;

namespace Editor
{		
	enum filter_type {
		NONE,
		BLUR,
		CONTOUR,
		EDGE,
		EMBOSS,
		SHARPEN,
		THRESHOLD
	};

	enum layer_type {
		FILTER,
		GROUP,
		IMAGE
	};

	class Layer {
		protected:
			bool active = true;				
			layer_type type;			
		public:
			Layer(layer_type type) {
				this->type = type;				
			}

			void set_active(bool active) {
				this->active = active;		
			}

			bool get_active() {
				return this->active;
			}						

			layer_type get_type() {
				return this->type;
			}

			virtual bool __process(cv::Mat* img) {
				if(!this->active)
					return false;

				return true;
			}
	};

	class FilterLayer : public Layer{		
		public:
			int* intensity = new int(0);
			filter_type* filter = new filter_type(NONE);						
			FilterLayer() : Layer(FILTER) {};			

			virtual bool __process(cv::Mat* img) {
				if(!Layer::__process(img))
					return false;

				if(!*this->intensity)
					return false;

				cv::Mat kernel;
				cv::Mat gray, edges;
				switch (*this->filter) {
					case BLUR:
						cv::blur(*img, *img, cv::Size(*this->intensity + 1, *this->intensity + 1));
						break;
					case CONTOUR:
						cv::Canny(*img, *img, *this->intensity, *this->intensity * 3);
						break;
					case EDGE:
						cv::Laplacian(*img, *img, CV_16S, *this->intensity);
						cv::convertScaleAbs(*img, *img);
						break;
					case EMBOSS:
						kernel = (cv::Mat_<float>(3, 3) << -2, -1, 0, -1, 1, 1, 0, 1, 2);
						cv::filter2D(*img, *img, CV_32F, kernel);
						cv::convertScaleAbs(*img, *img);
						break;
					case SHARPEN:
						kernel = (cv::Mat_<float>(3, 3) << 0, -1, 0, -1, *this->intensity + 5, -1, 0, -1, 0);
						cv::filter2D(*img, *img, CV_32F, kernel);
						cv::convertScaleAbs(*img, *img);
						break;
					case THRESHOLD:
						cv::cvtColor(*img, gray, cv::COLOR_RGBA2GRAY);
						cv::threshold(gray, *img, *this->intensity, 255, cv::THRESH_BINARY);
						cv::cvtColor(*img, *img, cv::COLOR_GRAY2RGBA);
						break;
					default:
						break;
				}

				return true;
			}
	};	

  class ImageLayer : public Layer {
		private:			
			cv::Mat img;	
			cv::Mat display_img;
			std::string file;								
		public:		
			int *pos_x = new int(0);
			int *pos_y = new int(0);
			int *width = new int(0);
			int *height = new int(0);

			ImageLayer(std::string file = "") : Layer(IMAGE) {
				if(!file.empty()) {					
					this->img = cv::imread(file, cv::IMREAD_COLOR);
					this->file = file;
					cv::cvtColor(this->img, this->img, cv::COLOR_BGR2RGBA);

					*this->width = this->img.cols;
					*this->height = this->img.rows;

					*this->pos_x = 0;
					*this->pos_y = 0;

					this->display_img = this->img.clone();
				}
			}

			~ImageLayer() {
				this->img.release();
			}

			cv::Mat get_image() {
				if(!*this->width)
					*this->width = 1;

				if(!*this->height)
					*this->height = 1;													

				if(*this->width != this->img.cols || *this->height != this->img.rows) {
					this->display_img.release();
					cv::resize(this->img, this->display_img, cv::Size(*this->width, *this->height));				
				}
				
				return this->display_img;
			}			

			std::string get_file() {
				return this->file;
			}
	};

	class GroupLayer : public Layer {
		private:
			std::map<int, std::shared_ptr<Layer>> layers;
			cv::Mat img;
			cv::Mat display_img;
			GLuint texture_id;
		public:		
			GroupLayer() : Layer(GROUP) {
				this->img = cv::Mat(work_height, work_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
			};

			void add(layer_type type) {                
		    switch (type) {
	        case FILTER:
            this->layers[this->layers.size()] = std::make_shared<FilterLayer>();
            break;            
	        case GROUP:
            this->layers[this->layers.size()] = std::make_shared<GroupLayer>();
            break;                    
	        case IMAGE:
            this->layers[this->layers.size()] = std::make_shared<ImageLayer>(_dialog_file());
            break;                                                
		    }                                                
			}

			GLuint get() {
				cvmat_to_gluint(this->display_img, &this->texture_id);
				return this->texture_id;
			}

			void list(void (*callback)(int, Layer*, void*), void *data = nullptr) {												
				for(auto it = layers.rbegin(); it != layers.rend(); ++it) {
					callback(it->first, it->second.get(), data);
				}
			}

			void remove(int pos) {
				layers.erase(pos);
			}

			bool process() {										 				
				ImageLayer* image_layer;			
				this->display_img.release();	
				this->display_img = this->img.clone();

				for(auto& _layer : layers) {					
					if(!_layer.second->get_active())
						continue;

					try {
						switch (_layer.second->get_type()) {
							case IMAGE:								
								image_layer = dynamic_cast<ImageLayer*>(_layer.second.get());									

								if(image_layer->get_file().empty())
									continue;

								if(this->img.empty()) {
									this->img = image_layer->get_image().clone();
									continue;
								}

								overlay(this->display_img, image_layer->get_image(), *image_layer->pos_x, *image_layer->pos_y);																
								break;
							default:							
								_layer.second->__process(&this->display_img);
								break;
						}					
					} catch (const std::exception e) {
						std::cerr << "Err: " << e.what() << std::endl;
					}								
				}

				return true;
			}			

			int size() {
				return this->layers.size();
			}			
	};
}
