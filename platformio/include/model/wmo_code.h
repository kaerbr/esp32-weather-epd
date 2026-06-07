#pragma once

/**
 * @brief Comprehensive WMO 4680 (wawa) / 4677 Weather Code Specification
 * Style: LLVM/Clang CamelCase Scoped Enum
 * Range 00-99 covers standard meteorology. Values 100+ are free for custom overrides.
 */
enum class WmoCode : uint8_t
{
  // 00-03: No Precipitation (Sky state)
  ClearSky                            = 0,
  MainlyClear                         = 1,
  PartlyCloudy                        = 2,
  Overcast                            = 3,

  // 04-09: Reduced Visibility / Aerosols
  SmokeOrAsh                          = 4,
  Haze                                = 5,
  WidespreadDustSuspension            = 6,
  DustOrSandRaisedByWind              = 7,
  DustWhirls                          = 8,
  DistantDustStorm                    = 9,

  // 10-12: Mist & Shallow Phenomena
  Mist                                = 10,
  ShallowFog                          = 11,
  LightningVisibleNoThunder           = 12,

  // 13-19: Severe Convective Features
  SquallsDistant                      = 13,
  ThunderstormNoPrecipitation         = 17,
  SevereSquallsAtStation              = 18,
  SevereTornadoOrFunnel               = 19,

  // 30-39: Wind, Dust, or Snow Storms
  DuststormSlightToModerate           = 30,
  DuststormSevere                     = 33,
  DriftingSnowLowLevel                = 36,
  BlowingSnowHighLevel                = 38,

  // 40-49: Fog or Ice Fog
  FogDistant                          = 40,
  Fog                                 = 45,
  FogDepositingRime                   = 48,

  // 50-59: Drizzle
  DrizzleLight                        = 51,
  DrizzleModerate                     = 53,
  DrizzleDense                        = 55,
  DrizzleFreezingLight                = 56,
  DrizzleFreezingDense                = 57,

  // 60-69: Rain
  RainSlight                          = 61,
  RainModerate                        = 63,
  RainHeavy                           = 65,
  RainFreezingLight                   = 66,
  RainFreezingHeavy                   = 67,
  RainSnowMixSlight                   = 68,
  RainSnowMixHeavy                    = 69,

  // 70-79: Solid Precipitation (Snow & Ice crystals)
  SnowSlight                          = 71,
  SnowModerate                        = 73,
  SnowHeavy                           = 75,
  SnowGrains                          = 77,
  IceCrystals                         = 78,

  // 80-89: Showers (Convective Precipitation)
  ShowersRainSlight                   = 80,
  ShowersRainModerate                 = 81,
  ShowersRainViolent                  = 82,
  ShowersRainSnowMixSlight            = 83,
  ShowersRainSnowMixHeavy             = 84,
  ShowersSnowSlight                   = 85,
  ShowersSnowHeavy                    = 86,
  ShowersIcePellets                   = 87,
  ShowersHailNoThunder                = 89,

  // 90-99: Thunderstorms
  ThunderstormSlightOrModerate        = 95,
  ThunderstormHailSlight              = 96,
  ThunderstormHeavyNoHail             = 97,
  ThunderstormHailHeavy               = 99,

  Unknown                             = 255
};