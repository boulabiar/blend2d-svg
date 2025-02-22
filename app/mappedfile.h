#ifndef MAPPEDFILE_H
#define MAPPEDFILE_H

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <memory>
#include <cerrno>
#include <cstdio>

namespace waavs
{
    struct MappedFile
    {
        void* fData{nullptr};
        size_t fSize{0};
        bool fIsValid{false};
        int fFileDescriptor{-1};

    public:
        MappedFile(int fd, void* data, size_t size) noexcept
            : fData(data), fSize(size), fIsValid(true), fFileDescriptor(fd)
        {}

        MappedFile() noexcept = default;

        ~MappedFile() noexcept { close(); }

        bool isValid() const noexcept { return fIsValid; }
        void* data() const noexcept { return fData; }
        size_t size() const noexcept { return fSize; }

        bool close() noexcept
        {
            bool success = true;
            if (fData != nullptr)
            {
                if (munmap(fData, fSize) != 0)
                    success = false;
                fData = nullptr;
            }

            if (fFileDescriptor != -1)
            {
                if (::close(fFileDescriptor) != 0)
                    success = false;
                fFileDescriptor = -1;
            }

            fIsValid = false;
            return success;
        }

        static std::shared_ptr<MappedFile> create_shared(const std::string& filename,
            int oflags = O_RDONLY,
            mode_t mode = 0666) noexcept
        {
            int fd = ::open(filename.c_str(), oflags, mode);
            if (fd == -1)
            {
                perror("MappedFile::create_shared (open)");
                return {};
            }

            struct stat sb;
            if (fstat(fd, &sb) == -1)
            {
                perror("MappedFile::create_shared (fstat)");
                ::close(fd);
                return {};
            }

            size_t size = sb.st_size;

            void* data = mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
            if (data == MAP_FAILED)
            {
                perror("MappedFile::create_shared (mmap)");
                ::close(fd);
                return {};
            }

            return std::make_shared<MappedFile>(fd, data, size);
        }
    };
}

#endif // MAPPEDFILE_H
