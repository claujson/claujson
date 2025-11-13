#pragma once

#include "claujson.h"
namespace claujson {
	// class PartialJson, only used in class LoadData.
	class PartialJson {
	protected:
		my_vector<_Value> arr_vec;
		//
		my_vector<Pair<_Value, _Value>> obj_data;

		_Value virtualJson = _Value();

		Arena* pool;

		static _Value data_null; // valid is false..
		static const uint64_t npos; // 
	public:
		~PartialJson();

	private:
		friend class LoadData;
		friend class LoadData2;
		friend class Object;
		friend class Array;
		friend class StructuredPtr;

		PartialJson(Arena* pool);

	public:
		bool is_partial_json() const;

		bool is_object() const;

		bool is_array() const;

		uint64_t get_data_size() const;

		_Value& get_value_list(uint64_t idx);

		Arena* get_pool() {
			return pool;
		}

		bool has_pool() const {
			return pool;
		}
	private:
		_Value& get_key_list(uint64_t idx);
	public:

		const _Value& get_value_list(uint64_t idx) const;


		const _Value& get_key_list(uint64_t idx) const;

		const _Value& get_const_key_list(uint64_t idx);

		const _Value& get_const_key_list(uint64_t idx) const;

		void clear(uint64_t idx);

		bool is_virtual() const;

		void clear();

		void reserve_data_list(uint64_t len);

		bool add_object_element(Value key, Value val);
		bool add_array_element(Value val);

	public:
		void MergeWith(Array* j, int start_offset);
		void MergeWith(Object* j, int start_offset);
		void MergeWith(PartialJson* j, int start_offset);

	private:
		void add_item_type(int64_t key_buf_idx, int64_t key_next_buf_idx, int64_t val_buf_idx, int64_t val_next_buf_idx,
			char* buf, uint64_t key_token_idx, uint64_t val_token_idx);
		
		void add_item_type(int64_t val_buf_idx, int64_t val_next_buf_idx,
			char* buf, uint64_t val_token_idx);

		void add_user_type(int64_t key_buf_idx, int64_t key_next_buf_idx, char* buf,
			_ValueType type, uint64_t key_token_idx, Arena* pool

		);
		void add_user_type(_ValueType type, Arena* pool

		);
	};


}
