int fm_build(fm_index *index, uchar *text, ulong length);

int fm_read_file(char *filename, uchar **textt, ulong *length);
int fm_read_file2(char *filename, uchar **textt, ulong *length, int stripnewlines);
int fm_build_config (fm_index *index, suint tc,  suint freq, 
							ulong bsl1, ulong bsl2, suint owner);
