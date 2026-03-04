#include "RingBuffer.h"

// --- Method to write data into the ring buffer, returns the number of bytes actually written ---
size_t RingBuffer::Write(const uint8_t* data, size_t len)
{
    std::lock_guard<std::mutex> lock(mtx);
    size_t written = 0;

    while (written < len && count < buffer.size())
    {
        buffer[head] = data[written++];
        head = (head + 1) % buffer.size();
        ++count;
    }
    return written;
}

// --- Method to read data from the ring buffer without removing it, returns the number of bytes actually peeked ---
size_t RingBuffer::Peek(uint8_t* data, size_t len) const
{
    std::lock_guard<std::mutex> lock(mtx);
    size_t peeked = 0;
    size_t tempTail = tail;
    while (peeked < len && peeked < count)
    {
        data[peeked++] = buffer[tempTail];
        tempTail = (tempTail + 1) % buffer.size();
    }
	return peeked;
}

// --- Method to read data from the ring buffer, returns the number of bytes actually read ---
size_t RingBuffer::Read(uint8_t* data, size_t len)
{
    std::lock_guard<std::mutex> lock(mtx);
    size_t read = 0;

    while (read < len && count > 0)
    {
        data[read++] = buffer[tail];
        tail = (tail + 1) % buffer.size();
        --count;
    }
    return read;
}

// --- Method to check how many bytes are currently available in the buffer for reading ---
size_t RingBuffer::Available() const
{
    return count;
}

// --- Method to check how much free space is available in the buffer for writing ---
size_t RingBuffer::FreeSpace() const
{
    return buffer.size() - count;
}

// --- Method to clear the buffer (resets head, tail, and count) ---
void RingBuffer::Clear()
{
    std::lock_guard<std::mutex> lock(mtx);
    head = tail = count = 0;
}

// --- Method to get the total capacity of the buffer ---
size_t RingBuffer::Capacity() const
{
    return buffer.size();
}

bool RingBuffer::IsEmpty() const
{
    return count == 0;
}

// --- Method to check if the buffer is full ---
bool RingBuffer::IsFull() const
{
    return count == buffer.size();
}



