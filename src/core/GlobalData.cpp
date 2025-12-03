#include "BlkFxParam.h"
#include "VstProgram.h"
#include <vector>

std::vector<VstProgram<BlkFxParam::TOTAL_NUM>> g_blk_fx_presets;

bool GlobalInitOk()
{
  return true;
}
