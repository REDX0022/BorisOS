
extern void reset_queue();

extern void dir_enqueue(struct directory dir);

extern void dir_dequeue(struct directory* ret);

extern int dir_is_queue_empty();
