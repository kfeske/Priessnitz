struct Trace {
	unsigned knight_count;
};

Trace &trace();

static inline void trace_increment_knight_count()
{
	trace().knight_count++;
}
