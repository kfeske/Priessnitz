#pragma once

unsigned const DEFAULT_TT_SIZE = 128; // MB
unsigned const BUCKET_SIZE = 2;

struct TT_entry
{
	uint64_t key;
	uint8_t depth;
	int16_t evaluation;
	uint16_t best_move;
	uint8_t flag;
	uint8_t age; // Could be combined with flag.
};

struct TT_bucket
{
	TT_entry entries[BUCKET_SIZE];
};

struct Transposition_table
{
	unsigned entry_count = (DEFAULT_TT_SIZE * 1024 * 1024) / sizeof(TT_entry);
	unsigned bucket_count = entry_count / BUCKET_SIZE;
	TT_bucket *buckets;
	TT_entry empty_entry {};
	bool hit = false;

	TT_entry &probe(uint64_t key)
	{
		hit = true;
		uint64_t index = key % bucket_count;
		TT_bucket &bucket = buckets[index];
		for (unsigned i = 0; i < BUCKET_SIZE; i++) {
			TT_entry &entry = bucket.entries[i];
			if (entry.key == key) return entry;
		}
		hit = false;
		return empty_entry;
	}

	void store(uint64_t key, unsigned depth, int evaluation, Move best_move, TT_flag flag, unsigned age)
	{
		uint64_t index = key % bucket_count;
		TT_bucket &bucket = buckets[index];

		TT_entry &first_entry  = bucket.entries[0];
		TT_entry &second_entry = bucket.entries[1];
		if (depth >= first_entry.depth || age != first_entry.age) {
			second_entry = first_entry;
			first_entry.key = key;
			first_entry.depth = depth;
			first_entry.best_move = best_move;
			first_entry.flag = flag;
			first_entry.evaluation = int16_t(evaluation);
			first_entry.age = age;
		}
		else {
			second_entry.key = key;
			second_entry.depth = depth;
			second_entry.best_move = best_move;
			second_entry.flag = flag;
			second_entry.evaluation = int16_t(evaluation);
			second_entry.age = age;
		}
	}

	void resize(unsigned size_mb)
	{
		delete[] buckets;
		entry_count = (size_mb * 1024 * 1024) / sizeof(TT_entry);
		bucket_count = entry_count / BUCKET_SIZE;
		buckets = new TT_bucket[bucket_count];
	}

	Transposition_table()
	:
		 buckets(new TT_bucket[bucket_count])
	{}
};
