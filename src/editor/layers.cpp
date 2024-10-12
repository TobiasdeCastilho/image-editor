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
		THRESHOLD,
		GRAY
	};

	enum layer_type {
		FILTER,
		GROUP,
		IMAGE,
		POINT
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
					case GRAY:						
						cv::cvtColor(*img, gray, cv::COLOR_RGBA2GRAY);						
						break;					
					default:
						break;
				}

				return true;
			}
	};	

  class ImageLayer : public Layer {
		private:			
			cv::Mat display_img;
			std::string file;								
			int last_rotation = 0;			
			int last_width = 0;
			int last_height = 0;
			float last_scale = 0;
		public:		
			cv::Mat img;	
			int *pos_x = new int(0);
			int *pos_y = new int(0);
			int *width = new int(0);
			int *height = new int(0);
			int *rotation = new int(0);
			int *opacity = new int(100);
			float *scale = new float(1);

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

				if(*this->width != this->last_width || *this->height != this->last_height || *this->rotation != this->last_rotation || *this->scale != this->last_scale) {
					this->display_img.release();
					this->display_img = this->img.clone();
																				
					cv::resize(this->display_img, this->display_img, cv::Size(*this->width * (*this->scale), *this->height * (*this->scale)));
					cv::Point2f center(this->display_img.cols / 2.0, this->display_img.rows / 2.0);			
					cv::Mat rot_mat = cv::getRotationMatrix2D(center, *this->rotation, 1.0);					
					cv::warpAffine(this->display_img, this->display_img, rot_mat, this->display_img.size());					

					cv::RotatedRect rotatedRect(center, cv::Size2f(*this->width, *this->height), *this->rotation);					
					cv::Point2f vertices[4];
					rotatedRect.points(vertices);
					std::vector<cv::Point> vertices_int;
					for (int i = 0; i < 4; ++i)
					  vertices_int.push_back(cv::Point(static_cast<int>(vertices[i].x), static_cast<int>(vertices[i].y)));															

					this->last_rotation = *this->rotation;
					this->last_width = *this->width;
					this->last_height = *this->height;				
					this->last_scale = *this->scale;	
				}				
				
				return this->display_img;
			}			

			std::string get_file() {
				return this->file;
			}			

			void reset(){				
				*this->width = this->img.cols;
				*this->height = this->img.rows;
				*this->rotation = 0;
				*this->opacity = 100;
				*this->scale = 1;
				*this->pos_x = 0;
				*this->pos_y = 0;
			}
	};

	struct _Point {
		int x, y;
	};
	typedef struct _Point Point;
	class PointLayer: public Layer {
		public:		
			Point points[4];
			float *scale = new float(1);
			int *rotation = new int(0);			

			PointLayer() : Layer(POINT) {
				for (int i = 0; i < 4; i++) {
					points[i].x = -1;
					points[i].y = -1;
				}
			}
			
			virtual bool __process(cv::Mat* img) {		    
		    cv::Point2f center(img->cols / 2.0f, img->rows / 2.0f);

				float pos_x = -1, pos_y = -1;
		    for (Point _point : points) {
	        if (_point.x == -1 && _point.y == -1)
	            continue;
	        
	        float scaled_x = _point.x * *this->scale;
	        float scaled_y = _point.y * *this->scale;

	        cv::Point2f scaled_point(scaled_x, scaled_y);

	        float rad = *this->rotation * CV_PI / 180.0;
	        float cos_theta = cos(rad);
	        float sin_theta = sin(rad);

	        float rotated_x = cos_theta * (scaled_point.x - center.x) - sin_theta * (scaled_point.y - center.y) + center.x;
	        float rotated_y = sin_theta * (scaled_point.x - center.x) + cos_theta * (scaled_point.y - center.y) + center.y;

	        cv::circle(*img, cv::Point(rotated_x, rotated_y), 5, cv::Scalar(0, 0, 255), -1);
					pos_x = rotated_x;
					pos_y = rotated_y;		    
					if(pos_x != -1 || pos_y != -1)
						cv::line(*img, cv::Point(rotated_x, rotated_y), cv::Point(rotated_x, rotated_y), cv::Scalar(0, 0, 255), 5);				
		    }

			return true;
		}

		void reset(){
			for (int i = 0; i < 4; i++) {
				points[i].x = -1;
				points[i].y = -1;
			}
			*rotation = 0;
			*scale = 1;
		}

	};	

	class GroupLayer : public Layer {
		private:
			std::map<int, std::shared_ptr<Layer>> layers;
			cv::Mat img;
			cv::Mat display_img;
			GLuint texture_id;
		public:		
			bool changed = false;

			GroupLayer() : Layer(GROUP) {
				this->img = cv::Mat(work_height, work_width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
			};

			~GroupLayer() {
				this->img.release();
				this->display_img.release();
			}

			void add(layer_type type) {                
		    std::string file = "";
				
				switch (type) {
	        case FILTER:
            this->layers[this->layers.size()] = std::make_shared<FilterLayer>();
            break;            
	        case GROUP:
            this->layers[this->layers.size()] = std::make_shared<GroupLayer>();
            break;                    					
	        case IMAGE:					
						file = _dialog_file();
						if(!file.empty())	
            	this->layers[this->layers.size()] = std::make_shared<ImageLayer>(file);
            break;                    
					case POINT:						
						this->layers[this->layers.size()] = std::make_shared<PointLayer>();
						break;
	        default:
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
				layers.at(pos).reset();
				layers.erase(pos);

				int i = 0;
				for(auto& _layer : layers) {
					layers[i] = _layer.second;
					i++;
				}
			}

			bool process() {				
				//if(!this->changed)
				//	return false;

				this->changed = false;

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

								overlay(this->display_img, image_layer->get_image(), *image_layer->pos_x, *image_layer->pos_y, *image_layer->opacity);																
								break;						
							case POINT:
								_layer.second->__process(&this->display_img);
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

			void swap(int a, int b) {
				if(a < 0 || a >= (int)this->layers.size() || b < 0 || b >= (int)this->layers.size())
					return;				

				std::swap(this->layers[a], this->layers[b]);
			}
	};
}
