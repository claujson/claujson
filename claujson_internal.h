#pragma once

#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include <list>
#include <string>
#include <set>
#include <fstream>
#include <cstring>
#include <cstdint> // uint64_t? int64_t?


template <class From, class To>
inline To Static_Cast(From x) {
	To temp = static_cast<To>(x);
	bool valid = static_cast<From>(temp) == x;
	if (!valid) {
		throw std::runtime_error("static cast error");
	}
	return temp;
}



#if __cpp_lib_string_view
#include <string_view>
using namespace std::literals::string_view_literals;
namespace claujson {
	using StringView = std::string_view;
}

#else

namespace claujson {
	class StringView {
	public:
		explicit StringView() : m_str(nullptr), m_len(0) {}

		StringView(const std::string& str) : m_str(str.data()), m_len(str.size()) {}
		explicit StringView(const char* str) : m_str(str) { m_len = strlen(str); }
		explicit StringView(const char* str, uint64_t len) : m_str(str), m_len(len) {}
		StringView(const StringView& other) {
			m_str = other.m_str;
			m_len = other.m_len;
		}

	public:
		const char* data() const {
			return m_str;
		}

		uint64_t size() const {
			return m_len;
		}

		uint64_t length() const {
			return m_len;
		}

		bool empty() const {
			return 0 == m_len;
		}

		StringView substr(uint64_t pos, uint64_t n) const {
			return StringView(m_str + pos, n);
		}

		const char& operator[](uint64_t idx) const {
			return m_str[idx];
		}

		// returns index;
		uint64_t find(const char ch, uint64_t start = 0) {
			for (uint64_t i = start; i < size(); ++i) {
				if (ch == (*this)[i]) {
					return i;
				}
			}
			return npos;
		}

		StringView& operator=(const StringView& other) {
			StringView temp(other);
			this->m_str = temp.m_str;
			this->m_len = temp.m_len;
			return *this;
		}
	private:
		const char* m_str;
		uint64_t m_len;
	public:
		static const uint64_t npos;

		friend std::ostream& operator<<(std::ostream& stream, const claujson::StringView& sv) {
			stream << sv.data();
			return stream;
		}

		bool operator==(const StringView view) {
			return this->compare(view) == 0;
		}

		bool operator!=(const StringView view) {
			return this->compare(view) != 0;
		}

		int compare(const StringView view) {
			uint64_t idx1 = 0, idx2 = 0;
			for (; idx1 < this->length() && idx2 < view.length(); ++idx1, ++idx2) {
				uint8_t diff = this->data()[idx1] - view.data()[idx2];
				if (diff < 0) {
					return -1;
				}
				else if (diff > 0) {
					return 1;
				}
			}
			if (idx1 < this->length()) {
				return 1;
			}
			else if (idx2 < view.length()) {
				return -1;
			}
			return 0;
		}

		bool operator<(const StringView view) {
			return this->compare(view) < 0;
		}
	};
}

claujson::StringView operator""sv(const char* str, uint64_t sz);
bool operator==(const std::string& str, claujson::StringView sv);



#endif



namespace claujson {

	// has buf?
	template <class T, int SIZE = 1024>
	class Vector {
	public:
		class iterator {
		private:
			T* ptr;
		public:
			iterator(T* now) {
				ptr = now;
			}
		public:
			void operator++() {
				++ptr;
			}
			T* operator->() {
				return ptr;
			}
			const T* operator->() const {
				return ptr;
			}
			T& operator*() {
				return *ptr;
			}
			const T& operator*() const {
				return *ptr;
			}
			bool operator!=(const iterator& other) const {
				return this->ptr != other.ptr;
			}
		};
		class const_iterator {
		private:
			T* ptr;
		public:
			const_iterator(T* now) {
				ptr = now;
			}
		public:
			const T* operator->() const {
				return ptr;
			}
			const T& operator*() const {
				return *ptr;
			}
			bool operator!=(const const_iterator& other) const {
				return this->ptr != other.ptr;
			}
		};
	private:
		T buf[SIZE + 1];
		T* ptr = nullptr;
		uint32_t capacity = SIZE;
		uint32_t sz = 0;
		int type = 0;
	public:
		Vector() {
			//
		}
		~Vector() {
			if (type == 1 && ptr) {
				delete[] ptr;
				ptr = nullptr;
				sz = 0;
				type = 0;
			}
		}
		Vector(const Vector&) = delete;
		Vector& operator=(const Vector&) = delete;
		Vector(Vector&& other) noexcept {
			std::swap(ptr, other.ptr);
			std::swap(capacity, other.capacity);
			std::swap(sz, other.sz);
			std::swap(type, other.type);
			memcpy(buf, other.buf, SIZE * sizeof(T));
		}
		Vector& operator=(Vector&& other) noexcept {
			Vector temp(std::move(other));

			std::swap(this->ptr, temp.ptr);
			std::swap(this->capacity, temp.capacity);
			std::swap(this->sz, temp.sz);
			std::swap(this->type, temp.type);
			memcpy(this->buf, temp.buf, SIZE * sizeof(T));

			return *this;
		}
	public:
		iterator begin() {
			return iterator(type == 1 ? ptr : buf);
		}
		const_iterator begin() const {
			return const_iterator(type == 1 ? ptr : buf);
		}
		iterator end() {
			return iterator(type == 1 ? ptr + sz : buf + SIZE);
		}
		const_iterator end() const {
			return const_iterator(type == 1 ? ptr + sz : buf + SIZE);
		}
	public:
		void clear() {
			if (type == 1 && ptr) {
				delete[] ptr;
				ptr = nullptr;
			}
			capacity = SIZE;
			sz = 0;
			type = 0;
		}

		T& back() {
			if (type == 0) {
				return buf[sz - 1];
			}
			return ptr[sz - 1];
		}
		const T& back() const {
			if (type == 0) {
				return buf[sz - 1];
			}
			return ptr[sz - 1];
		}
		void pop_back() {
			if (empty() == false) {
				--sz;
			}
		}
		bool empty() const { return 0 == sz; }
		uint64_t size() const { return sz; }
		void push_back(T val) {
			if (type == 0) {
				buf[sz] = std::move(val);
				++sz;
				if (sz == SIZE) {
					if (ptr) {
						delete[] ptr; ptr = nullptr;
					}
					ptr = new (std::nothrow) T[SIZE * 2];
					if (!ptr) {
						throw ("new failed");
					}
					capacity = SIZE * 2;
					memcpy(ptr, buf, SIZE * sizeof(T));
					type = 1;
				}
			}
			else {
				if (sz < capacity) {
					ptr[sz] = std::move(val);
					++sz;
				}
				else {
					T* temp = new (std::nothrow) T[2 * capacity];
					if (!temp) {
						throw ("new failed");
					}
					memcpy(temp, ptr, sz * sizeof(T));
					capacity = capacity * 2;
					delete[] ptr;
					ptr = temp;
					ptr[sz] = std::move(val);
					++sz;
				}
			}
		}
	public:
		T& operator[](const uint64_t idx) {
			if (type == 0) {
				return buf[idx];
			}
			return ptr[idx];
		}
		const T& operator[](const uint64_t idx) const {
			if (type == 0) {
				return buf[idx];
			}
			return ptr[idx];
		}

	};

	class Log;

	template <class T>
	static void _print(Log& log, const T& val, const int op);

	class Log {
	public:
		class Info {
		public:
			friend std::ostream& operator<<(std::ostream& stream, const Info&) {
				stream << "[INFO] ";
				return stream;
			}
		};
		class Warning {
		public:
			friend std::ostream& operator<<(std::ostream& stream, const Warning&) {
				stream << "[WARN] ";
				return stream;
			}
		};

		enum class Option { CONSOLE, FILE, CONSOLE_AND_FILE, NO_PRINT };
		class Option2 {
		public:
			static const int INFO = 1;
			static const int WARN = 2;
			static const int CLEAR = 0;
		};
	private:
		Option opt; // console, file, ...
		int opt2; // info, warn, ...
		int state; // 1 : info, 2 : warn. // default is info!
		std::string fileName;
	public:

		Log() : state(0), opt(Option::NO_PRINT), opt2(Option2::CLEAR), fileName("log.txt") {
			//
		}

	public:
		template <class T>
		friend void _print(Log& log, const T& val, const int op);

	public:

		Option option() const {
			return opt;
		}

		int option2() const {
			return opt2;
		}

		void console() {
			opt = Option::CONSOLE;
		}

		void file() {
			opt = Option::FILE;
		}

		void console_and_file() {
			opt = Option::CONSOLE_AND_FILE;
		}

		void no_print() {
			opt = Option::NO_PRINT;
			opt2 = Option2::CLEAR;
		}

		void file_name(const std::string& str) {
			fileName = str;
		}

		void info(bool only = false) {
			if (only) {
				opt2 = Option2::INFO;
			}
			else {
				opt2 = opt2 | Option2::INFO;
			}
		}
		void warn(bool only = false) {
			if (only) {
				opt2 = Option2::WARN;
			}
			else {
				opt2 = opt2 | Option2::WARN;
			}
		}
	};

	template <class T>
	static void _print(Log& log, const T& val, const int op) { // op : change_state, with op.

		if (op == 0 || op == 1) {
			log.state = op;
		}

		if (log.opt == Log::Option::CONSOLE || log.opt == Log::Option::CONSOLE_AND_FILE) {

			int count = 0;

			if ((log.opt2 & Log::Option2::INFO) && log.state == 0) {
				count = 1;
			}
			if ((log.opt2 & Log::Option2::WARN) && log.state == 1) {
				count = 1;
			}

			if (count) {
				std::cout << val;
			}
		}

		if (log.opt == Log::Option::FILE || log.opt == Log::Option::CONSOLE_AND_FILE) {
			std::ofstream outFile;
			outFile.open(log.fileName, std::ios::app);
			if (outFile) {
				int count = 0;

				if ((log.opt2 & Log::Option2::INFO) && log.state == 0) {
					count = 1;
				}
				if ((log.opt2 & Log::Option2::WARN) && log.state == 1) {
					count = 1;
				}

				if (count) {
					outFile << val;
				}
				outFile.close();
			}
		}
	}

	template <class T>
	inline Log& operator<<(Log& log, const T& val) {
		_print(log, val, -1);
		return log;
	}

	template<>
	inline Log& operator<<(Log& log, const Log::Info& x) {
		_print(log, x, 0);
		return log;
	}
	template<>
	inline Log& operator<<(Log& log, const Log::Warning& x) {
		_print(log, x, 1);
		return log;
	}

	extern Log::Info info;
	extern Log::Warning warn;
	extern Log log; // no static..
	// inline Error error;

	template <class T>
	using PtrWeak = T*;

	template <class T>
	using Ptr = std::unique_ptr<T>;
	// Ptr - use std::move


	enum class _ValueType : int32_t {
		NONE = 0, // chk 
		ARRAY, // ARRAY_OBJECT -> ARRAY, OBJECT
		OBJECT,
		PARTIAL_JSON,
		INT, UINT,
		FLOAT,
		BOOL,
		NULL_,
		STRING, SHORT_STRING,
		NOT_VALID,
		ERROR // private class?
	};

	template <class Key, class Data>
	class Pair {
	public:
		Key first = Key();
		Data second = Data();
	public:
		Pair() {}
		Pair(Key&& first, Data&& second) : first(std::move(first)), second(std::move(second)) {}
		Pair(const Key& first, Data&& second) : first((first)), second(std::move(second)) {}
		Pair(Key&& first, const Data& second) : first(std::move(first)), second((second)) {}
	};

	// todo - smartpointer? std::unique<Block> ?
	template <class Block>
	class BlockManager { // manager for not using?
	public: // or friend? or set_~~ 
		// start_block-> ...->last_block(or nullptr)
		Block* start_block = nullptr;
		Block* last_block = nullptr;
	public:
		[[nodiscard]]
		Block* Get(uint64_t cap) {
			if (!start_block) {
				start_block = new (std::nothrow)Block(cap);
				if (start_block == nullptr) { return nullptr; }
				Block* result = start_block;
				start_block = nullptr;
				return result;
			}
			Block* block = start_block;
			Block* before_block = nullptr;
	
			while (block && (block->capacity != cap)) {
				before_block = block;
				block = block->next;
			}
			if (block) {
				if (before_block) {
					before_block->next = block->next;
				}
				else { // 
					start_block = block->next;
					if (start_block == nullptr) {
						last_block = nullptr;
					}
				}
				block->offset = 0;
				block->next = nullptr;
				return block;
			}
			return new(std::nothrow)Block(cap);
		}
	public:
		BlockManager(Block* start_block = nullptr, Block* last_block = nullptr) : start_block(start_block), last_block(last_block) {
			if (last_block) {
				last_block->next = nullptr;
			}
		}

		// check last_block?, has bug?
		void RemoveBlocks() {
			//t64_t count = 0;
			Block* block = start_block;
			while (block) {
				Block* next = block->next;
				delete block;
				block = next;
				//unt++;
			}	//d::cout << "count1 " << count << " \n";
		}
	};

	// bug - 크기를 줄일떄? 메모리 소비?
	// memory_pool?
	class Arena {
	public:
		struct Block {
			Block* next; //
			uint64_t capacity;
			uint64_t offset;
			uint8_t* data;
			
			Block(uint64_t cap)
				: next(nullptr), capacity(cap), offset(0) {
				//data = (uint8_t*)mi_malloc(sizeof(uint8_t) * capacity); // 
				data = new (std::nothrow) uint8_t[capacity];
			}

			~Block() {
				delete[] data;
				data = nullptr;
			//	mi_free(data);
			}

			Block(const Block&) = delete;
			Block& operator=(const Block&) = delete;
		};
	public:
		Block* head[2];
		Block* rear[2];
		const uint64_t defaultBlockSize;
		Arena* now_pool;
		Arena* next;
		uint64_t count = 0;
	public:
		static const uint64_t initialSize = 1024 * 1024; // 4 * 1024?

	private:
		BlockManager<Block> blockManager[2];
		std::vector<Block*> startBlockVec[2];
		std::vector<Block*> lastBlockVec[2];

	private:
		void RemoveBlocks(int no) {
			{
				Clear(no);
				std::vector<BlockManager<Block>> result = DivideBlock(no);
				for (auto& x : result) {
					x.RemoveBlocks();
				}
			}

			blockManager[no].RemoveBlocks();
			Block* block = head[no];
			//t count = 0;
			while (block) {
				Block* next = block->next;
				delete block;
				block = next;
				//unt++;
			}
			//d::cout << "count2 " << count << " \n";
		}

		void Clear(int no) {
			if (lastBlockVec[no].empty()) {
				if (blockManager[no].last_block) {
					blockManager[no].last_block->next = head[no];
				}
				else {
					blockManager[no].start_block = head[no];
				}
				if (!blockManager[no].start_block) {
					blockManager[no].start_block = head[no];
				}

				blockManager[no].last_block = rear[no];
				if (blockManager[no].last_block) {
					blockManager[no].last_block->next = nullptr;
				}

				head[no] = blockManager[no].Get(defaultBlockSize);
				rear[no] = head[no];
			}
			else {
				Reset(no);
			}
		}

		// link_from -> Reset -> DivideBlock -> link_from...
		void Reset(int no) {
			if (lastBlockVec[no].empty()) {
				return;
			}
			if (blockManager[no].last_block) {
				blockManager[no].last_block->next = head[no];
			}
			else {
				blockManager[no].start_block = head[no];
			}
			if (!blockManager[no].start_block) {
				blockManager[no].start_block = head[no];
			}

			blockManager[no].last_block = lastBlockVec[no][0];
			if (blockManager[no].last_block) {
				blockManager[no].last_block->next = nullptr;
			}

			head[no] = blockManager[no].Get(defaultBlockSize);
			rear[no] = head[no];
		}

 		std::vector<BlockManager<Block>> DivideBlock(int no) {
			if (lastBlockVec[no].empty()) { return {}; }
			lastBlockVec[no].push_back(nullptr);

			blockManager[no].last_block = lastBlockVec[no][0];
			if (blockManager[no].last_block) {
				blockManager[no].last_block->next = nullptr;
			}

			std::vector<BlockManager<Block>> blocks;
			for (uint64_t i = 1; i < lastBlockVec[no].size(); ++i) {
				blocks.push_back({ startBlockVec[no][i - 1], lastBlockVec[no][i] });
			}

			startBlockVec[no].clear();
			lastBlockVec[no].clear();
	
			return blocks;
		}
	public:
		void RemoveBlocks() {
			RemoveBlocks(0);
			RemoveBlocks(1);
		}
		void Clear() {
			Clear(0);
			Clear(1);
			now_pool = this;
			// chk! memory leak.-fix
			while (next) {
				Arena* temp = next->next;
				next->next = nullptr;
				delete next;
				next = temp;
			}
			next = nullptr;
		}
		void Reset() {
			Reset(0);
			Reset(1);
			//now_pool = this;
			// chk! memory leak.-fix
			while (next) {
				Arena* temp = next->next;
				next->next = nullptr;
				delete next;
				next = temp;
			}
			next = nullptr;
		}
		std::vector<std::vector<BlockManager<Block>>> DivideBlock() {
			std::vector<std::vector<BlockManager<Block>>> result;
			result.push_back(DivideBlock(0));
			result.push_back(DivideBlock(1));
			return result;
		}

	public:
		Arena(uint64_t size = initialSize) : defaultBlockSize(size) {
			for (int i = 0; i < 2; ++i) { // i == 0 4k, i == 1  > 4k
				blockManager[i] = BlockManager<Block>(nullptr, nullptr);
				head[i] = blockManager[i].Get(defaultBlockSize);
				rear[i] = head[i];
			}
			now_pool = this;
			next = nullptr;
		}
		Arena(Block* start_block0, Block* last_block0, Block* start_block1, Block* last_block1, uint64_t size = initialSize)
			: defaultBlockSize(size) {
			blockManager[0] = BlockManager<Block>(start_block0, last_block0);
			blockManager[1] = BlockManager<Block>(start_block1, last_block1);

			for (int i = 0; i < 2; ++i) { // i < 2
				head[i] = blockManager[i].Get(defaultBlockSize); // (new (std::nothrow) Block(initialSize));
				rear[i] = head[i];
			}
			now_pool = this;
			next = nullptr;
		}

		Arena(const Arena&) = delete;
		Arena& operator=(const Arena&) = delete;

		static int64_t counter;
	public:
		template <class T>
		T* allocate(uint64_t size, uint64_t align = alignof(T)) {
			{
				int no = 0;
				if (size + 64 >= defaultBlockSize) {
					no = 1;
				}
				Block* block = now_pool->head[no];

				if (block) {
					if (block->offset + size < block->capacity) {
						uint64_t remain = block->capacity - block->offset;

						void* ptr = block->data + block->offset;
						void* aligned_ptr = ptr;
						
						if (std::align(alignof(T), size, aligned_ptr, remain)) {
							size_t aligned_offset = static_cast<uint8_t*>(aligned_ptr) - block->data;

							block->offset = aligned_offset + size;

							return reinterpret_cast<T*>(aligned_ptr);
						}
					}
					block = block->next;
					
					//break;
				}
			}

			// allocate new block
			uint64_t newCap = std::max(defaultBlockSize, size + 64);
			int no = 0;
			if (newCap == size + 64) {
				no = 1;
			}
			Block* newBlock = blockManager[no].Get(newCap); // new (std::nothrow) Block(newCap);
			if (!newBlock) {
				return nullptr;
			}
			if (newCap == size + 64) { // chk over size?
				counter++;
			}

			uint64_t remain = newBlock->capacity - newBlock->offset;
			void* ptr = newBlock->data + newBlock->offset;
			void* aligned_ptr = ptr;

			if (std::align(alignof(T), size, aligned_ptr, remain)) {
				uint64_t aligned_offset = static_cast<uint8_t*>(aligned_ptr) - newBlock->data;

				newBlock->offset = aligned_offset + size;

				newBlock->next = now_pool->head[no];

				now_pool->head[no] = newBlock;
				
				return reinterpret_cast<T*>(aligned_ptr);
			}

			return nullptr;
		}

		// expand
		template <class T>
		void deallocate(T* ptr, uint64_t len) {		


			return;

			//return;
			
			// chk !
			Block* block = now_pool->head[0];

			while (block) {
				if ((uint8_t*)(ptr) + sizeof(T) * len == (uint8_t*)(block->data) + block->offset) {
					block->offset = (uint8_t*)ptr - (uint8_t*)block->data;
						//std::cout << "real_deallocated\n"; //
					return;
				}

				block = block->next;
			}
		}

		template<typename T, typename... Args>
		T* create(Args&&... args) {
			void* mem = allocate<T>(sizeof(T), alignof(T));
			return new (mem) T(std::forward<Args>(args)...);
		}

	public:
		~Arena() {
			if (this != now_pool) {
				return;
			}
			
			RemoveBlocks();

			while (next) {
				Arena* temp = next->next;
				next->next = nullptr;
				delete next;
				next = temp;
			}
		}

		// chk! when merge?
		void link_from(Arena* other) {
			for (int i = 0; i < 2; ++i) { // i < 1
				if (!this->head[i]) {
					this->head[i] = other->head[i];
					this->rear[i] = other->rear[i];
				}
				else {
					this->startBlockVec[i].push_back(other->head[i]);
					this->lastBlockVec[i].push_back(this->rear[i]);

					this->rear[i]->next = other->head[i];
					this->rear[i] = other->rear[i];
				}
			}

			for (int no = 0; no < 2; ++no) {
				if (this->blockManager[no].last_block) {
					this->blockManager[no].last_block->next = other->blockManager[no].start_block;
				}
				else {
					this->blockManager[no].start_block = other->blockManager[no].start_block;
				}
				if (!this->blockManager[no].start_block) {
					this->blockManager[no].start_block = other->blockManager[no].start_block;
				}
				this->blockManager[no].last_block = other->blockManager[no].last_block;

				other->blockManager[no].start_block = nullptr;
				other->blockManager[no].last_block = nullptr;
			}

			other->now_pool = this->now_pool;
			
			other->next = this->next;
			this->next = other;
		
			for (int i = 0; i < 2; ++i) {
				other->head[i] = nullptr;
				other->rear[i] = nullptr;
			}
		}
	};

	template <class T>
	class Vector2 {
	private:
		Arena* pool = nullptr;
		T* m_arr = nullptr;
		uint32_t m_capacity = 0;
		uint32_t m_size = 0;
	public:
		// =delete?
		Vector2() : pool(nullptr) {
			//
		}
		Vector2(uint64_t sz) : pool(nullptr) {
			m_size = sz;
			m_capacity = 2 * sz;
			m_arr = new T[m_capacity]();
		}
		// with Arena..
		Vector2(Arena* pool, uint64_t sz) : pool(pool) {
			m_size = sz;
			m_capacity = 2 * sz;
			if (pool) {
				m_arr = (T*)pool->allocate<T>(sizeof(T) * m_capacity, alignof(T));

				//for (uint64_t i = 0; i < m_size; ++i) {
				//	new (&m_arr[i]) T();
				//}
			}
			else {
				std::cout << "pool is nullptr\n"; // chk?
				m_arr = new T[m_capacity]();
			}
		}
		Vector2(Arena* pool, uint64_t sz, uint64_t capacity) : pool(pool) {
			m_size = sz;
			m_capacity = capacity; 
			// todo - sz <= capacity!
			if (pool) {
				m_arr = (T*)pool->allocate<T>(sizeof(T) * m_capacity, alignof(T));

				//for (uint64_t i = 0; i < m_size; ++i) {
				//	new (&m_arr[i]) T();
				//}
			}
			else {
				m_arr = new T[m_capacity]();
			}
		}
		~Vector2() {
			if (m_arr && !pool) { delete[] m_arr; }
			else if (m_arr) {
				//for (uint64_t i = 0; i < m_size; ++i) {
				//	m_arr[i].~T();
				//}

				if (m_capacity > 0) {
					pool->deallocate(m_arr, m_capacity);
				}
			}
		}
		Vector2(const Vector2& other) {
			if (other.m_arr) {
				this->pool = other.pool;
				if (pool) {
					this->m_arr = (T*)pool->allocate<T>(sizeof(T) * other.m_capacity, alignof(T));
				}
				else {
					this->m_arr = new T[other.m_capacity]();
				}
				//this->m_arr = new T[other.m_capacity];
				this->m_capacity = other.m_capacity;
				this->m_size = other.m_size; 
				for (uint64_t i = 0; i < other.m_size; ++i) {
					new (&m_arr[i]) T(other.m_arr[i]);
				}
			}
		}
		Vector2(Vector2&& other) noexcept {
			std::swap(m_arr, other.m_arr);
			std::swap(m_capacity, other.m_capacity);
			std::swap(m_size, other.m_size);
			std::swap(pool, other.pool);
		}
	public:
		Vector2& operator=(const Vector2& other) {
			if (this == &other) { return *this; }

			if (other.m_arr) {
				if (this->m_arr && !this->pool) {
					delete[] this->m_arr; this->m_arr = nullptr;
				}
				else if (this->m_arr) {
					for (uint64_t i = 0; i < m_size; ++i) {
						m_arr[i].~T();
					}
					this->pool->deallocate(this->m_arr, this->m_capacity);
				}
				if (this->pool) {
					this->m_arr = pool->allocate<T>(sizeof(T) * other.m_capacity, alignof(T));
				}
				else {
					this->m_arr = new T[other.m_capacity]();
				}
				this->m_capacity = other.m_capacity;
				this->m_size = other.m_size;
				for (uint64_t i = 0; i < other.m_size; ++i) {
					new (&m_arr[i]) T(other.m_arr[i]);
				}
			}
			else {
				if (m_arr && !this->pool) { delete[] m_arr; }
				else if (m_arr) {
					for (uint64_t i = 0; i < m_size; ++i) {
						m_arr[i].~T();
					}
					this->pool->deallocate(this->m_arr, this->m_capacity);
				}
				m_arr = nullptr;
				m_capacity = 0;
				m_size = 0;
			}
			return *this;
		}
		void operator=(Vector2&& other) noexcept {
			if (this == &other) { return; }

			std::swap(m_arr, other.m_arr);
			std::swap(m_capacity, other.m_capacity);
			std::swap(m_size, other.m_size);
			std::swap(pool, other.pool);
		}
	public:
		Arena* get_pool() { return pool; }

	public:

		// rename...! // [start_idx ~ size())
		[[nodiscard]]
		Vector2<T> Divide(uint64_t start_idx) {
			Vector2<T> result;

			if (pool && start_idx < size()) {
				result.pool = this->pool->now_pool;
				result.m_arr = this->m_arr + start_idx;
				result.m_capacity = (capacity() - start_idx);
				result.m_size = size() - start_idx;
				this->m_capacity = start_idx; // ?
				this->m_size = start_idx;
			}
			else if (start_idx < size()) {
				for (uint64_t i = start_idx; i < size(); ++i) {
					result.push_back(std::move(this->m_arr[i]));
				}
				for (uint64_t i = m_size; i > start_idx; --i) {
					this->pop_back();
				}
			}

			return result;
		}
		void erase(T* p) {
			uint64_t idx = p - m_arr;

			for (uint64_t i = idx; i + 1 < m_size; ++i) {
				m_arr[i] = std::move(m_arr[i + 1]);
			}

			m_size--;
		}
		bool has_pool()const {
			return pool;
		}
		void insert(T* start, T* last) {
			uint64_t sz = m_size + (last - start);

			if (sz <= m_size) { return; }

			{
				if (sz > m_capacity) {
					expand(2 * sz);
				} 
				for (uint64_t i = m_size; i < sz; ++i) {
					new (&m_arr[i]) T(std::move(start[i - m_size]));
				//	m_arr[i] = std::move(start[i - m_size]);
				}
			}
			m_size = sz;
		}

		void push_back(T x) {
			if (size() >= capacity()) {
				if (capacity() == 0) {
					expand(2);
				}
				else {
					expand(2 * capacity());
				}
			}

			//new (&m_arr[m_size]) T();
			new (&m_arr[m_size++]) T(std::move(x));
		}

		template<typename... _Args>
		void emplace_back(_Args&&... __args) {
			if (size() >= capacity()) {
				if (capacity() == 0) {
					expand(2);
				}
				else {
					expand(2 * capacity());
				}
			}
			new (&m_arr[m_size++]) T(std::forward<_Args>(__args)...);
		}


		void pop_back() {
			if (m_size > 0) {
				m_size--;
			}
		}
		T& back() {
			return m_arr[m_size - 1];
		}
		const T& back() const {
			return m_arr[m_size - 1];
		}
		T& operator[](uint64_t idx) {
			return m_arr[idx];
		}
		const T& operator[](uint64_t idx) const {
			return m_arr[idx];
		}
		void reserve(uint64_t sz) {
			if (capacity() < sz) {
				expand(sz);
			}
		}
		bool empty() const { return 0 == m_size; }
		T* begin() { return m_arr; }
		T* end() { return m_arr + m_size; }
		const T* begin() const { return m_arr; }
		const T* end() const { return m_arr + m_size; }
		void clear() {
			m_size = 0;
		}
		uint64_t size() const { return m_size; }
		uint64_t capacity() const { return m_capacity; }

		void resize(uint64_t sz) {
			if (sz < m_capacity) {
				m_size = sz;
				return;
			}
			expand(sz * 2);
			m_size = sz;
		}
	private:
		void expand(uint64_t new_capacity) {
			if (pool) {
				T* temp = (T*)pool->allocate<T>(sizeof(T) * new_capacity);
				for (uint64_t i = 0; i < m_size; ++i) {
					//new (temp + i) T();
					new (temp + i) T(std::move(m_arr[i]));
				}

				//for (uint64_t i = 0; i < m_size; ++i) {
				//	m_arr[i].~T();
				//}
				if (temp != m_arr) {
					pool->deallocate<T>(m_arr, m_capacity);
				}
				m_arr = temp;
			}
			else {
				T* temp = new (std::nothrow) T[new_capacity]();
				if (m_arr) {
					for (uint64_t i = 0; i < m_size; ++i) {
						temp[i] = std::move(m_arr[i]);
					}
					delete[] m_arr;
				}

				m_arr = temp;
			}
			m_capacity = new_capacity;
		}
	};

	template <class T>
	using my_vector = Vector2<T>;

} // end of claujson

