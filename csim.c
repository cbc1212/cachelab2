//student id 2023200418
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct CacheLine {
    int valid;
    int tag;
    int last_used_time;
} CacheLine;

typedef struct CacheSet {
    CacheLine* lines;
} CacheSet;

typedef struct Cache {
    CacheSet* sets;
    int s; // Set index bits
    int E; // Lines per set
    int b; // Block offset bits
} Cache;

unsigned hit = 0, miss = 0, eviction = 0;
int timestamp = 0;

void printSummary(unsigned hits, unsigned misses, unsigned evictions) {
    printf("hits:%u misses:%u evictions:%u\n", hits, misses, evictions);
    FILE* output_fp = fopen(".csim_results", "w");
    assert(output_fp);
    fprintf(output_fp, "%u %u %u\n", hits, misses, evictions);
    fclose(output_fp);
}

void printHelp(const char* name) {
    printf(
        "Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n"
        "Options:\n"
        "  -h         Print this help message.\n"
        "  -v         Optional verbose flag.\n"
        "  -s <num>   Number of set index bits.\n"
        "  -E <num>   Number of lines per set.\n"
        "  -b <num>   Number of block offset bits.\n"
        "  -t <file>  Trace file.\n\n"
        "Examples:\n"
        "  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n"
        "  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n",
        name, name, name);
}

void accessCache(Cache* cache, size_t address) {
    int set_index = (address >> cache->b) & ((1 << cache->s) - 1);
    int tag = address >> (cache->b + cache->s);

    CacheSet* current_set = &cache->sets[set_index];
    int lru_index = 0, lru_time = current_set->lines[0].last_used_time;

    for (int i = 0; i < cache->E; ++i) {
        if (current_set->lines[i].valid && current_set->lines[i].tag == tag) {
            ++hit;
            current_set->lines[i].last_used_time = timestamp;
            return;
        }
        if (current_set->lines[i].last_used_time < lru_time) {
            lru_time = current_set->lines[i].last_used_time;
            lru_index = i;
        }
    }
    ++miss;
    eviction += (lru_time != -1);
    current_set->lines[lru_index].last_used_time = timestamp;
    current_set->lines[lru_index].tag = tag;
    current_set->lines[lru_index].valid = 1;
}

int main(int argc, char* argv[]) {
    int s = 0, E = 0, b = 0;
    char* trace_file_name = NULL;
    int verbose = 0;

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 's':
                    s = atoi(argv[++i]);
                    break;
                case 'E':
                    E = atoi(argv[++i]);
                    break;
                case 'b':
                    b = atoi(argv[++i]);
                    break;
                case 't':
                    trace_file_name = argv[++i];
                    break;
                case 'h':
                    printHelp(argv[0]);
                    return 0;
                case 'v':
                    verbose = 1;
                    break;
            }
        }
    }

    if (!trace_file_name) {
        printHelp(argv[0]);
        return 1;
    }

    Cache cache;
    cache.s = s;
    cache.E = E;
    cache.b = b;
    cache.sets = (CacheSet*)calloc((1 << s), sizeof(CacheSet));
    for (int i = 0; i < (1 << s); ++i) {
        cache.sets[i].lines = (CacheLine*)calloc(E, sizeof(CacheLine));
        for (int j = 0; j < E; ++j) {
            cache.sets[i].lines[j].valid = 0;
            cache.sets[i].lines[j].last_used_time = -1;
        }
    }

    FILE* trace_file = fopen(trace_file_name, "r");
    if (!trace_file) {
        printf("Error: Could not open file %s\n", trace_file_name);
        return 1;
    }

    char operation;
    size_t address;
    int memsize, register_name;

    while (fscanf(trace_file, " %c %lx,%d %d", &operation, &address, &memsize, &register_name) == 4) {
        ++timestamp;
        if (operation == 'L' || operation == 'S') {
            accessCache(&cache, address);
        }
    }

    for (int i = 0; i < (1 << s); ++i) {
        free(cache.sets[i].lines);
    }
    free(cache.sets);

    printSummary(hit, miss, eviction);
    fclose(trace_file);
    return 0;
}
