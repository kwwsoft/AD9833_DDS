#pragma once
#include <vector>
#include <mutex>

class RingBuffer
{
	public:
		// --- Constructor to initialize the ring buffer with a specified capacity (default is 32 KB) ---
        explicit RingBuffer(size_t capacity = 32768) : buffer(capacity) {}

		// --- Method to write data into the ring buffer, returns the number of bytes actually written ---
        size_t Write(const uint8_t* data, size_t len);

		// прочитати дані з буфера без видалення їх, повертає кількість фактично прочитаних байтів
		size_t Peek(uint8_t* data, size_t len) const;
		
        // --- Method to read data from the ring buffer, returns the number of bytes actually read ---
		size_t Read(uint8_t* data, size_t len);
		
        // --- Method to check how many bytes are currently available in the buffer for reading ---
        size_t Available() const;
        
        // --- Method to check how much free space is available in the buffer for writing ---
        size_t FreeSpace() const;
        
        // --- Method to clear the buffer (resets head, tail, and count) ---
        void Clear();
        
        // --- Method to get the total capacity of the buffer ---
        size_t Capacity() const; 
 		
        // --- Method to check if the buffer is empty ---
        bool IsEmpty() const; 

		// --- Method to check if the buffer is full ---
        bool IsFull() const;

private:
    std::vector<uint8_t> buffer;
    size_t head = 0;
    size_t tail = 0;
    size_t count = 0;
    mutable std::mutex mtx;
};

