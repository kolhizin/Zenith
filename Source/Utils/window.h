#pragma once

#include <Windows.h>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include "nameid.h"
#include "wndUtil.h"
#include "log_config.h"

#undef max
#undef ERROR

namespace zenith
{
	namespace util
	{
		class Window;

		class WindowException : public zenith::util::LoggedException
		{
		public:
			WindowException() : zenith::util::LoggedException() {}
			WindowException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(p, type)
			{
			}
			WindowException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(str, type)
			{
			}
			virtual ~WindowException() {}
		};

		class IWindowMessageHandler
		{
		public:
			virtual ~IWindowMessageHandler() {}
			virtual void process(Window * wnd, WPARAM wparam, LPARAM lparam) {};
		};
		template<class T, class Func>
		class WindowMessageHandlerT : public IWindowMessageHandler
		{
			T * pData_;
			Func fn_;
		public:
			WindowMessageHandlerT(const Func &fn, T * pData) : fn_(fn), pData_(pData) {}
			virtual void process(Window * wnd, WPARAM wparam, LPARAM lparam)
			{
				fn_(pData, wnd, wparam, lparam);
			}
		};
		template<class Func>
		class WindowMessageHandler : public IWindowMessageHandler
		{	
			Func fn_;
		public:
			WindowMessageHandler(const Func &fn) : fn_(fn) {}
			virtual void process(Window * wnd, WPARAM wparam, LPARAM lparam)
			{
				fn_(wnd, wparam, lparam);
			}
		};

		template<class Func, class T>
		class WindowMessageHandlerWithData : public IWindowMessageHandler
		{
			Func fn_;
			T data_;
		public:
			WindowMessageHandlerWithData(const Func &fn, T &&data) : fn_(fn), data_(std::move(data)) {}
			virtual void process(Window * wnd, WPARAM wparam, LPARAM lparam)
			{
				fn_(data_, wnd, wparam, lparam);
			}
		};

		template<class Func>
		std::unique_ptr<IWindowMessageHandler> makeWindowMessageHandler(const Func &f)
		{
			return std::make_unique<WindowMessageHandler<Func>>(f);
		}

		template<class Func, class T>
		std::unique_ptr<IWindowMessageHandler> makeWindowMessageHandler(T &&data, const Func &f)
		{
			return std::make_unique<WindowMessageHandlerWithData<Func, T>>(f, std::move(data));
		}

		class WindowImpl_
		{
			friend class Window;
			Window * owner_;
			volatile bool selfDestruct_;
			std::multimap<UINT, std::pair<nameid, std::unique_ptr<IWindowMessageHandler>> > handlers_;
			HWND wnd_;
			HICON icon_;
			bool customIcon_;

			static LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				static std::map<HWND, WindowImpl_ *> actors;
				static HWND lastDestroyed_ = 0;
				if (uMsg == WM_CREATE)
				{
					LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT)lParam;
					WindowImpl_ * ptr = (WindowImpl_ *)pCreateStruct->lpCreateParams;
					actors[hwnd] = ptr;
				}
				auto iter = actors.find(hwnd);
				LRESULT res = 0;
				if (iter == actors.end())
					return DefWindowProc(hwnd, uMsg, wParam, lParam);
				WindowImpl_ * ptr = iter->second;
				if (uMsg == WM_DESTROY || ptr->selfDestruct_)
				{
					actors.erase(iter);
					delete ptr;
					lastDestroyed_ = hwnd;
				}
				else
					ptr->windowProc(uMsg, wParam, lParam);
				if (lastDestroyed_ == hwnd)
					return res;
				if (ptr->selfDestruct_)
				{
					actors.erase(iter);
					delete ptr;
					lastDestroyed_ = hwnd;
				}
				return res;
			}
			inline LRESULT windowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
			
			void addHandler(UINT msg, nameid name, std::unique_ptr<IWindowMessageHandler> &&ptr)
			{
				handlers_.insert(std::make_pair(msg, std::make_pair(name, std::move(ptr))));
			}
			void removeHandler(nameid name)
			{
				while (true)
				{
					bool found = false;
					for(auto it = handlers_.begin(); it != handlers_.end(); it++)
						if (it->second.first == name)
						{
							handlers_.erase(it);
							found = true;
							break;
						}
					if (!found)
						break;
				}
			}

			void clearHandlers()
			{
				handlers_.clear();
			}
			bool peekMessage()
			{
				MSG msg = { 0 };
				if (PeekMessage(&msg, wnd_, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					return true;
				}
				return false;
			}



			WindowImpl_(const WndConfig &conf, Window * owner) : customIcon_(false), selfDestruct_(false)
			{
				owner_ = owner;

				zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "WindowImpl_: Starting window creation.");

				WNDCLASSEX wcex;
				wcex.cbSize = sizeof(wcex);
				wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
				wcex.lpfnWndProc = (WNDPROC)wndProc;
				wcex.cbClsExtra = 0;
				wcex.cbWndExtra = 0;
				wcex.hInstance = 0;
				if (conf.iconFilename != "")
				{
					icon_ = (HICON)LoadImageA(NULL, conf.iconFilename.c_str(), IMAGE_ICON, SM_CXICON, SM_CYICON, LR_LOADFROMFILE);
					customIcon_ = true;
				}else icon_ = LoadIcon(NULL, IDI_APPLICATION);
				wcex.hIcon = icon_;
				wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
				wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
				wcex.lpszMenuName = NULL;
				wcex.lpszClassName = "Default Window";
				wcex.hIconSm = icon_;

				zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "WindowImpl_: Registering window class.");

				if (!RegisterClassEx(&wcex))
					throw WindowException("Could not create window!");

				if (conf.fullscreen)
				{
					/*
					DEVMODE dmScreenSettings;
					memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
					dmScreenSettings.dmSize = sizeof(dmScreenSettings);
					dmScreenSettings.dmPelsWidth = traits.width;
					dmScreenSettings.dmPelsHeight = traits.height;
					dmScreenSettings.dmBitsPerPel = 32;
					dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
					if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
						throw std::exception("Could change screen settings!");
						*/
					//wnd_ = CreateWindowExA(WS_EX_APPWINDOW/*WS_EX_OVERLAPPEDWINDOW*/, "Default Window", traits.wndName.c_str(),
					//	/*WS_OVERLAPPEDWINDOW*/WS_POPUP, traits.x, traits.y, traits.width, traits.height, NULL, NULL, NULL, this);
				}
				else
				{
					zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "WindowImpl_: Creating non-fullscreen window.");
					wnd_ = CreateWindowExA(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, "Default Window", conf.title.c_str(),
						WS_OVERLAPPEDWINDOW, conf.pos.x, conf.pos.y, conf.size.width, conf.size.height, nullptr, nullptr, nullptr, this);
				}
				if (!wnd_)
					throw WindowException("Could not create window!");

				zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "WindowImpl_: Window created.");
				ShowWindow(wnd_, SW_SHOW);
				ShowCursor(conf.showCursor);
				SetCapture(wnd_);
				zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "WindowImpl_: Window showed & captured. Creation finished.");
			}
			~WindowImpl_()
			{
				zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "WindowImpl_: Destroying window & resources.");
				if (customIcon_)
					DestroyIcon(icon_);
				DestroyWindow(wnd_);
				zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "WindowImpl_: Window destroyed.");
			}

			WndSize getSize() const
			{
				RECT rect;
				if (!GetClientRect(wnd_, &rect))
					throw WindowException("Window::getSize: could not perform GetClientRect function!");
				WndSize sz;
				sz.width = static_cast<uint16_t>(rect.right - rect.left);
				sz.height = static_cast<uint16_t>(rect.bottom - rect.top);
				return sz;
			}
		};

		class Window
		{
			WindowImpl_ * impl_;
			WndConfig conf_;
			volatile bool loopOnline_;
			inline void destroy_()
			{
				if (impl_)
				{
					impl_->owner_ = nullptr;
					impl_->selfDestruct_ = true;
				}
				impl_ = nullptr;
			}
			inline void create_(const WndConfig &conf)
			{
				if (impl_)
					destroy_();
				conf_ = conf;
				impl_ = new WindowImpl_(conf_, this);
			}
			inline WindowImpl_ * getImpl_()
			{
				if (!impl_)
					throw WindowException("WindowImpl is null!");
				return impl_;
			}
			inline const WindowImpl_ * getImpl_() const
			{
				if (!impl_)
					throw WindowException("WindowImpl is null!");
				return impl_;
			}
		public:
			static WndSize getWndSize(const Window * p) { return p->getSize(); }

			Window() : impl_(nullptr) {}
			Window(const WndConfig &conf) : impl_(nullptr)
			{
				create_(conf);
			}
			~Window()
			{
				destroy_();
			}
			inline void stopLoop() { loopOnline_ = false; }
			inline void destroy()
			{
				loopOnline_ = false;
				destroy_();
			}
			inline void create(const WndConfig &conf)
			{
				destroy_();
				create_(conf);
			}
			inline WndSize getSize() const
			{
				return getImpl_()->getSize();
			}

			inline void addHandler(UINT msg, nameid name, std::unique_ptr<IWindowMessageHandler> &&ptr)
			{
				getImpl_()->addHandler(msg, name, std::move(ptr));
			}
			HWND hwnd() const
			{
				return getImpl_()->wnd_;
			}
			template<class T>
			void loop(T func)
			{
				loopOnline_ = true;
				while (impl_ && loopOnline_)
					if (!impl_->peekMessage() && loopOnline_)
						func();
			}
		};

		inline LRESULT WindowImpl_::windowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			if (uMsg == WM_DESTROY || selfDestruct_)
			{
				if (owner_)
					owner_->destroy();
				return 0;
			}
			auto iter_rng = handlers_.equal_range(uMsg);
			if (iter_rng.first == iter_rng.second)
				return DefWindowProc(wnd_, uMsg, wParam, lParam);
			for (auto it = iter_rng.first; it != iter_rng.second; it++)
				it->second.second->process(owner_, wParam, lParam);
			return 0;
		}
	}
}