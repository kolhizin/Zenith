#pragma once

#include "../Threading/task_queue.h"
#include <memory>
#include <map>

namespace zenith
{
	namespace util
	{
		namespace io
		{

			struct FileResult
			{
				uint8_t * data; //owned by caller-thread
				size_t size;
			};
						
			
			enum class FileStatus {UNDEF = 0, OPEN_READ, OPEN_WRITE};
			enum class FileOperation { UNDEF = 0, OPEN_READ, OPEN_WRITE, CLOSE, READ, WRITE, SIZE_LEFT };
			enum class FileInternalOp_ { UNDEF = 0, FILE_OP, DESTROY };

			struct FileOperationInfo
			{
				FileOperation operation;
				uint8_t * data;
				size_t size;
			};
			
			class FileImpl_
			{
				FILE * f;
				FileStatus status_;

				FileImpl_(const FileImpl_ &) = delete;
				FileImpl_ &operator =(const FileImpl_ &) = delete;
				FileImpl_ &operator =(FileImpl_ &&) = delete;

			public:
				FileImpl_(FileImpl_ &&fi) : f(fi.f), status_(fi.status_)
				{
					fi.f = nullptr;
					fi.status_ = FileStatus::UNDEF;
				}
				FileImpl_() : status_(FileStatus::UNDEF), f(nullptr) {}
				~FileImpl_()
				{
					if (status_ == FileStatus::UNDEF)
						return;
					fclose(f);
				}

				inline void execute(const FileOperationInfo &in, FileResult &out)
				{
					out.data = in.data;
					out.size = 0;
					if (in.operation == FileOperation::OPEN_READ || in.operation == FileOperation::OPEN_WRITE)
					{
						if (status_ != FileStatus::UNDEF)
							throw std::logic_error("FileImpl_::execute(OPEN): file is already open!");
						f = nullptr;
						if(in.operation == FileOperation::OPEN_READ)
							fopen_s(&f, reinterpret_cast<const char *>(in.data), "rb");
						else if (in.operation == FileOperation::OPEN_WRITE)
							fopen_s(&f, reinterpret_cast<const char *>(in.data), "wb");
						if (f != nullptr)
						{
							if (in.operation == FileOperation::OPEN_READ)
								status_ = FileStatus::OPEN_READ;
							if (in.operation == FileOperation::OPEN_WRITE)
								status_ = FileStatus::OPEN_WRITE;
							return;
						}
						else
							throw std::runtime_error("FileImpl_::execute(OPEN): failed to open file!");
					}
					if (in.operation == FileOperation::CLOSE)
					{
						if (status_ != FileStatus::OPEN_READ && status_ != FileStatus::OPEN_WRITE)
							throw std::logic_error("FileImpl_::execute(CLOSE): file is not open!");
						if(fclose(f) != 0)
							throw std::runtime_error("FileImpl_::execute(OPEN): failed to close file!");
						f = nullptr;
						status_ = FileStatus::UNDEF;
						return;
					}
					if (in.operation == FileOperation::READ)
					{
						if(status_ != FileStatus::OPEN_READ)
							throw std::logic_error("FileImpl_::execute(READ): file is not open in read-mode!");
						out.size = fread(out.data, 1, in.size, f);
						return;
					}
					if (in.operation == FileOperation::WRITE)
					{
						if (status_ != FileStatus::OPEN_WRITE)
							throw std::logic_error("FileImpl_::execute(WRITE): file is not open in write-mode!");
						out.size = fwrite(in.data, 1, in.size, f);
						return;
					}
					if (in.operation == FileOperation::SIZE_LEFT)
					{
						if (status_ != FileStatus::OPEN_READ)
							throw std::logic_error("FileImpl_::execute(SIZE_LEFT): file is not open in read-mode!");
						auto curPos = ftell(f);
						fseek(f, 0, SEEK_END);
						auto endPos = ftell(f);
						fseek(f, curPos, SEEK_SET);
						out.size = endPos - curPos;
					}
				}
			};

			class FileSystem;

			class FileHandle
			{
				friend class FileSystem;
				FileImpl_ * impl_;
				FileSystem * fs_;
				size_t queueId_;

				FileHandle(FileImpl_ * p, FileSystem * fs, size_t qid) : impl_(p), fs_(fs), queueId_(qid){}
			public:
				FileHandle(FileHandle &&f) : impl_(f.impl_), fs_(f.fs_), queueId_(f.queueId_)
				{
					f.fs_ = nullptr;
					f.impl_ = nullptr;
				}
				FileHandle &operator =(FileHandle &&f)
				{
					impl_ = f.impl_;
					fs_ = f.fs_;
					queueId_ = f.queueId_;
					f.impl_ = nullptr;
					f.fs_ = nullptr;
					return *this;
				}
				inline ~FileHandle();

				inline std::future<FileResult> open(const char * fName, char openType, float priority=1.0f);
				inline std::future<FileResult> read(uint8_t * pData, size_t maxSize, float priority = 1.0f);
				inline std::future<FileResult> write(uint8_t * pData, size_t size, float priority = 1.0f);
				inline std::future<FileResult> size_left(float priority = 1.0f);
				inline std::future<FileResult> close(float priority = 1.0f);
			};
			
			

			class FileSystem
			{
				friend class FileHandle;
				struct FileTaskInfo_
				{
					FileInternalOp_ opType;
					FileOperationInfo opInfo;
					FileImpl_ * impl;
					FileSystem * fs;
				};

				std::list<std::pair<FileImpl_, size_t>> files_;

				mutable std::mutex filesMutex_;

				std::vector<thread::task_queue<FileTaskInfo_, FileResult, 1>> taskQs_;
				size_t qIndexCounter_;

				static FileResult execute_(FileTaskInfo_ &tInfo)
				{					
					FileResult res = {};
					if (tInfo.opType == FileInternalOp_::FILE_OP)
					{
						tInfo.impl->execute(tInfo.opInfo, res);
						return res;
					}
					if (tInfo.opType == FileInternalOp_::DESTROY)
					{
						tInfo.fs->destroyFile_(tInfo.impl);
						return res;
					}
					return res;
				}
				void destroyFile_(FileImpl_ * p)
				{
					std::lock_guard<std::mutex> lk(filesMutex_);
					for (auto iter = files_.begin(); iter != files_.end(); iter++)
						if (&iter->first == p)
						{
							files_.erase(iter);
							return;
						}
				}

				std::future<FileResult> addFileOpTask_(float priority, FileHandle &h, const FileOperationInfo &foi)
				{
					FileTaskInfo_ taskInfo;
					taskInfo.fs = this;
					taskInfo.impl = h.impl_;
					taskInfo.opType = FileInternalOp_::FILE_OP;
					taskInfo.opInfo = foi;
					return taskQs_[h.queueId_].push(priority, std::move(taskInfo), execute_);
				}
				std::future<FileResult> destroyFile_(FileHandle &h)
				{
					FileTaskInfo_ taskInfo;
					taskInfo.fs = this;
					taskInfo.impl = h.impl_;
					taskInfo.opType = FileInternalOp_::DESTROY;
					return taskQs_[h.queueId_].push(0.0f, std::move(taskInfo), execute_);
				}
			public:
				FileSystem(size_t sz) : qIndexCounter_(0), taskQs_(sz)
				{

				}
				FileHandle createFile()
				{
					std::lock_guard<std::mutex> lk(filesMutex_);
					size_t ind = qIndexCounter_++;
					if (qIndexCounter_ >= taskQs_.size())
						qIndexCounter_ = 0;
					files_.emplace_back(std::make_pair(FileImpl_(), ind));
					return FileHandle(&files_.back().first, this, ind);
				}
			};


			inline FileHandle::~FileHandle()
			{
				if (fs_)
					fs_->destroyFile_(*this);
				fs_ = nullptr;
				impl_ = nullptr;
			}


			inline std::future<FileResult> FileHandle::open(const char * fName, char openType, float priority)
			{
				FileOperationInfo foi = {};
				if (openType == 'r')
					foi.operation = FileOperation::OPEN_READ;
				else if (openType == 'w')
					foi.operation = FileOperation::OPEN_WRITE;
				else throw std::logic_error("FileHandle::open: unknown openType!");

				foi.data = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(fName));
				foi.size = 0;

				return fs_->addFileOpTask_(priority, *this, std::move(foi));
			}
			inline std::future<FileResult> FileHandle::read(uint8_t * pData, size_t maxSize, float priority)
			{
				FileOperationInfo foi = {};
				foi.operation = FileOperation::READ;

				foi.data = pData;
				foi.size = maxSize;

				return fs_->addFileOpTask_(priority, *this, std::move(foi));
			}
			inline std::future<FileResult> FileHandle::write(uint8_t * pData, size_t size, float priority)
			{
				FileOperationInfo foi = {};
				foi.operation = FileOperation::WRITE;

				foi.data = pData;
				foi.size = size;

				return fs_->addFileOpTask_(priority, *this, std::move(foi));
			}
			inline std::future<FileResult> FileHandle::size_left(float priority)
			{
				FileOperationInfo foi = {};
				foi.operation = FileOperation::SIZE_LEFT;

				return fs_->addFileOpTask_(priority, *this, std::move(foi));
			}
			inline std::future<FileResult> FileHandle::close(float priority)
			{
				FileOperationInfo foi = {};
				foi.operation = FileOperation::CLOSE;

				return fs_->addFileOpTask_(priority, *this, std::move(foi));
			}
		}
	}
}
