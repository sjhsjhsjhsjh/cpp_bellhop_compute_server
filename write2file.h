#include "read_output.h"
#include "bellhop_params.h"
#include "read_npz.h"

void write_tl2file(PointPair& pair, const TLField& tlField);

void write_param2env(const PointPair& pair, bhc::bhcParams<true>& params);