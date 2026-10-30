#pragma once
namespace mc {
struct RadioPreset { const char *name; double frequency, bandwidth; int sf, cr, hashBytes; };
inline constexpr RadioPreset radioPresets[] = {
  {"Australia",915.800,250,10,5,1},
  {"Australia (Narrow)",916.575,62.5,7,7,1},
  {"Australia (Mid)",915.075,125,9,5,1},
  {"Australia: SA, WA",923.125,62.5,8,8,1},
  {"Australia: QLD",923.125,62.5,8,5,1},
  {"Brazil",923.125,62.5,8,8,1},
  {"Canada",910.525,62.5,7,5,3},
  {"Costa Rica",910.525,125,11,5,1},
  {"EU/UK (Narrow)",869.618,62.5,8,8,1},
  {"EU/UK (Deprecated)",869.525,250,11,5,1},
  {"Czech Republic (Narrow)",869.432,62.5,7,5,2},
  {"EU 433MHz (Long Range)",433.650,250,11,5,1},
  {"EU 433MHz (Narrow)",433.650,62.5,8,8,1},
  {"Hungary",869.618,62.5,7,5,2},
  {"Netherlands",869.618,62.5,7,5,1},
  {"Netherlands (Limburg)",869.618,62.5,8,8,2},
  {"New Zealand (Narrow)",917.375,62.5,7,5,2},
  {"New Zealand (Gisborne)",917.375,250,11,5,1},
  {"Portugal 433",433.375,62.5,9,6,1},
  {"Portugal 868",869.618,62.5,7,6,1},
  {"Slovakia",869.618,62.5,7,5,2},
  {"Switzerland",869.618,62.5,8,8,1},
  {"USA",910.525,62.5,7,5,1},
  {"USA - Southern California",927.875,62.5,7,5,3},
  {"Vietnam (Narrow)",920.250,62.5,8,5,1},
  {"Vietnam (Deprecated)",920.250,250,11,5,1},
};
inline constexpr int radioPresetCount = sizeof(radioPresets)/sizeof(radioPresets[0]);
}
