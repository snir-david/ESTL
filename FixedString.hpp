//
// Created by SnirN on 3/25/2025.
//

#ifndef ESTL_FIXEDSTRING_HPP
#define ESTL_FIXEDSTRING_HPP
#pragma once

#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace ESTL {

    // Base class containing common logic
    template<typename Derived>
    class FixedStringBase {
    protected:
        char *m_data;          // Pointer to the character buffer
        std::size_t m_size;    // Current size of the string
        std::size_t m_capacity;// Fixed capacity (for both CT and RT)

    public:
        // Constructor
        FixedStringBase(char *buffer, std::size_t capacity)
            : m_data(buffer)
            , m_size(0)
            , m_capacity(capacity) {
            m_data[0] = '\0';
        }

        // Size and capacity
        std::size_t size() const { return m_size; }
        std::size_t capacity() const { return m_capacity; }
        bool empty() const { return m_size == 0; }

        // Access
        char &operator[](std::size_t index) {
            if (index >= m_size) {
                throw std::out_of_range("Index out of range");
            }
            return m_data[index];
        }

        const char &operator[](std::size_t index) const {
            if (index >= m_size) {
                throw std::out_of_range("Index out of range");
            }
            return m_data[index];
        }

        const char *c_str() const { return m_data; }

        // Clear
        void clear() {
            m_size = 0;
            m_data[0] = '\0';
        }

        // Append
        void append(const char *str) {
            std::size_t len = std::strlen(str);
            if (m_size + len > m_capacity) {
                throw std::out_of_range("Exceeds fixed capacity");
            }
            std::strcat(m_data, str);
            m_size += len;
        }

        void append(const FixedStringBase &other) { append(other.c_str()); }

        char& front() {return m_data[0]; }

        char& back() {return m_data[m_size-1]; }

        void push_back(char c){
            if(m_size == m_capacity){
                throw std::out_of_range("Exceeds fixed capacity");
            }
            m_data[m_size] = c;
            m_size++;
        }

        void pop_back(){
            if(m_size == 0){
                throw std::out_of_range("Empty string");
            }
            m_data[m_size-1] = '\0';
            m_size--;
        }

        // Erase
        void erase(std::size_t pos, std::size_t len) {
            if (pos >= m_size) {
                throw std::out_of_range("Position out of range");
            }
            if (pos + len > m_size) {
                len = m_size - pos;
            }
            std::memmove(m_data + pos, m_data + pos + len, m_size - pos - len + 1);
            m_size -= len;
        }

        // Insert
        void insert(std::size_t pos, const char *str) {
            std::size_t len = std::strlen(str);
            if (pos > m_size || m_size + len > m_capacity) {
                throw std::out_of_range("Position out of range or exceeds fixed capacity");
            }
            std::memmove(m_data + pos + len, m_data + pos, m_size - pos + 1);
            std::memcpy(m_data + pos, str, len);
            m_size += len;
        }

        // Replace
        void replace(std::size_t pos, std::size_t len, const char *str) {
            erase(pos, len);
            insert(pos, str);
        }

        // Find
        std::size_t find(const char *str, std::size_t pos = 0) const {
            const char *found = std::strstr(m_data + pos, str);
            return found ? found - m_data : std::string::npos;
        }

        // RFind
        std::size_t rfind(const char *str, std::size_t pos = std::string::npos) const {
            if (pos == std::string::npos) {
                pos = m_size;
            }
            for (std::size_t i = pos; i != std::string::npos; --i) {
                if (std::strncmp(m_data + i, str, std::strlen(str)) == 0) {
                    return i;
                }
            }
            return std::string::npos;
        }

        // Starts with
        bool starts_with(const char *str) const {
            return std::strncmp(m_data, str, std::strlen(str)) == 0;
        }

        // Ends with
        bool ends_with(const char *str) const {
            std::size_t len = std::strlen(str);
            return m_size >= len && std::strncmp(m_data + m_size - len, str, len) == 0;
        }

        // Iterator class
        class iterator {
        private:
            char *m_ptr;

        public:
            explicit iterator(char *ptr) : m_ptr(ptr) {}

            char &operator*() const { return *m_ptr; }
            char *operator->() { return m_ptr; }

            iterator &operator++() {
                ++m_ptr;
                return *this;
            }

            iterator operator++(int) {
                iterator tmp = *this;
                ++m_ptr;
                return tmp;
            }

            iterator &operator--() {
                --m_ptr;
                return *this;
            }

            iterator operator--(int) {
                iterator tmp = *this;
                --m_ptr;
                return tmp;
            }

            bool operator==(const iterator &other) const { return m_ptr == other.m_ptr; }
            bool operator!=(const iterator &other) const { return m_ptr != other.m_ptr; }
        };

        iterator begin() { return iterator(m_data); }
        iterator end() { return iterator(m_data + m_size); }

        // Operators
        Derived &operator+=(const char *str) {
            append(str);
            return static_cast<Derived &>(*this);
        }

        Derived &operator+=(const FixedStringBase &other) { return (*this += other.c_str()); }

        bool operator==(const FixedStringBase &other) const { return std::strcmp(m_data, other.m_data) == 0; }

        bool operator!=(const FixedStringBase &other) const { return ! (*this == other); }

        friend std::ostream &operator<<(std::ostream &os, const FixedStringBase &str) {
            os << str.m_data;
            return os;
        }
    };

    // Compile-time FixedString
    template<std::size_t N>
    class CTString : public FixedStringBase<CTString<N>> {
    private:
        std::array<char, N + 1> m_buffer;// Null-terminated

    public:
        // Constructors
        CTString()
            : FixedStringBase<CTString<N>>(m_buffer.data(), N) {}

        CTString(const char *str)
            : CTString() {
            this->m_size = std::strlen(str);
            if (this->m_size > this->m_capacity) {
                throw std::out_of_range("String exceeds fixed capacity");
            }
            std::strcpy(this->m_data, str);
        }
    };

    // Runtime FixedString
    class RTString : public FixedStringBase<RTString> {
    public:
        // Constructor
        explicit RTString(std::size_t capacity)
            : FixedStringBase<RTString>(new char[capacity + 1], capacity) {}

        RTString(const char *str, std::size_t capacity)
            : RTString(capacity) {
            this->m_size = std::strlen(str);
            if (this->m_size > this->m_capacity) {
                throw std::out_of_range("String exceeds fixed capacity");
            }
            std::strcpy(this->m_data, str);
        }

        // Copy constructor
        RTString(const RTString &other)
            : FixedStringBase<RTString>(new char[other.m_capacity + 1], other.m_capacity) {
            this->m_size = other.m_size;
            std::strcpy(this->m_data, other.m_data);
        }

        // Assignment operator
        RTString &operator=(const RTString &other) {
            if (this != &other) {
                delete[] this->m_data;
                this->m_capacity = other.m_capacity;
                this->m_size = other.m_size;
                this->m_data = new char[this->m_capacity + 1];
                std::strcpy(this->m_data, other.m_data);
            }
            return *this;
        }

        // Destructor
        ~RTString() { delete[] this->m_data; }
    };

}// namespace ESTL

#endif//ESTL_FIXEDSTRING_HPP
