#pragma once

#include <map>
#include <opencv4/opencv2/opencv.hpp>

#include "../utils/conversion.cpp"

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
		IMAGE
	};

	class Layer {
		protected:
			bool active = true;				
			layer_type type;
			//ImageManager manager;
		public:
			Layer(layer_type type) {
				this->type = type;
				//this->manager = manager;
			}

			void set_active(bool active) {
				this->active = active;
				//this->manager.process_layers();
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
		private:						
			filter_type filter = NONE;			
		public:
			FilterLayer() : Layer(FILTER) {};			

			virtual bool __process(cv::Mat* img) {
				if(!Layer::__process(img))
					return false;

				cv::Mat kernel;
				switch (this->filter) {
					case BLUR:
						cv::blur(*img, *img, cv::Size(5, 5));
						break;
					case CONTOUR:
						cv::Canny(*img, *img, 100, 200);
						break;
					case EDGE:
						cv::Sobel(*img, *img, -1, 1, 1);
						break;
					case EMBOSS:
						kernel = (cv::Mat_<float>(3,3) << -2, -1, 0, -1, 1, 1, 0, 1, 2);
						cv::filter2D(*img, *img, -1, kernel);
						break;
					case SHARPEN:
						kernel = (cv::Mat_<float>(3,3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
						cv::filter2D(*img, *img, -1, kernel);
						break;
					case THRESHOLD:
						cv::threshold(*img, *img, 128, 255, cv::THRESH_BINARY);
						break;										
					default:
						break;
				}

				return true;
			}

			void set_filter(filter_type filter) {								
				this->filter = filter;				
			}									

			filter_type get_filter() {
				return this->filter;
			}						
	};	

	class ImageManager {
		private:
			std::map<std::string, std::shared_ptr<Layer>> layers;
			GLuint texture;
			cv::Mat img;							
		public:					
			ImageManager() {
				glGenTextures(1, &this->texture);
				glBindTexture(GL_TEXTURE_2D, this->texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				std::cout << "ImageManager created" << std::endl;
			}

			~ImageManager() {
				for(auto& layer : layers)
					layer.second.reset();
			}

			GLuint get_texture() {
				return this->texture;
			}

			void layers_add(layer_type type, std::string name, std::string file_path = "") {
				switch (type) {
					case FILTER:
						layers[name] = std::make_shared<FilterLayer>();
						break;			
					default:
						break;												
				}								

				this->process_layers();				
			}

			void layers_remove(const std::string name) {
				layers.erase(name);
				this->process_layers();				
			}

			void list_layers(void (*callback)(std::string, Layer* layer, int *count)) {
				int count = 0;
				
				for(auto& _layer : layers) {		
					switch (_layer.second->get_type()) {
						case FILTER: 
							callback(_layer.first, _layer.second.get(), &count);
							break;
						default:
							break;
					}				

					count++;
				}
			}

			void load_image(const std::string file_path) {
				this->img = cv::imread(file_path, cv::IMREAD_COLOR);
				cv::cvtColor(this->img, this->img, cv::COLOR_BGR2RGBA);				
				this->process_layers();
			}

			void process_layers() {				
				cv::Mat img = this->img.clone();

				for(auto& layer : layers)
					layer.second->__process(&img);

				cvmat_to_gluint(img, &this->texture);

				img.release();
			}
	};
}
