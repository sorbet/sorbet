# typed: strict

# Stored enum comparisons accumulate overlapping unions in control-flow knowledge.
# Computing their full intersections just to test emptiness is pathologically slow.
# Keep this workload fast by finding a shared enum value before constructing an intersection.
class Purpose < T::Enum
  enums do
    V0 = new
    V1 = new
    V2 = new
    V3 = new
    V4 = new
    V5 = new
    V6 = new
    V7 = new
    V8 = new
    V9 = new
    V10 = new
    V11 = new
    V12 = new
    V13 = new
    V14 = new
    V15 = new
    V16 = new
    V17 = new
    V18 = new
    V19 = new
    V20 = new
    V21 = new
    V22 = new
    V23 = new
    V24 = new
    V25 = new
    V26 = new
    V27 = new
    V28 = new
    V29 = new
    V30 = new
    V31 = new
    V32 = new
    V33 = new
    V34 = new
    V35 = new
    V36 = new
    V37 = new
    V38 = new
    V39 = new
    V40 = new
    V41 = new
    V42 = new
    V43 = new
    V44 = new
    V45 = new
    V46 = new
    V47 = new
    V48 = new
    V49 = new
    V50 = new
    V51 = new
    V52 = new
    V53 = new
    V54 = new
    V55 = new
    V56 = new
    V57 = new
    V58 = new
    V59 = new
    V60 = new
    V61 = new
    V62 = new
    V63 = new
    V64 = new
    V65 = new
    V66 = new
    V67 = new
    V68 = new
    V69 = new
    V70 = new
    V71 = new
    V72 = new
    V73 = new
    V74 = new
    V75 = new
    V76 = new
    V77 = new
    V78 = new
    V79 = new
    V80 = new
    V81 = new
    V82 = new
    V83 = new
    V84 = new
    V85 = new
    V86 = new
    V87 = new
    V88 = new
    V89 = new
    V90 = new
    V91 = new
    V92 = new
    V93 = new
    V94 = new
    V95 = new
    V96 = new
    V97 = new
    V98 = new
    V99 = new
    V100 = new
    V101 = new
    V102 = new
    V103 = new
    V104 = new
    V105 = new
    V106 = new
    V107 = new
    V108 = new
    V109 = new
    V110 = new
    V111 = new
    V112 = new
    V113 = new
    V114 = new
    V115 = new
    V116 = new
    V117 = new
    V118 = new
    V119 = new
    V120 = new
    V121 = new
    V122 = new
    V123 = new
    V124 = new
    V125 = new
    V126 = new
    V127 = new
    V128 = new
    V129 = new
    V130 = new
    V131 = new
    V132 = new
    V133 = new
    V134 = new
    V135 = new
    V136 = new
    V137 = new
    V138 = new
    V139 = new
    V140 = new
    V141 = new
    V142 = new
    V143 = new
    V144 = new
    V145 = new
    V146 = new
    V147 = new
    V148 = new
    V149 = new
    V150 = new
    V151 = new
    V152 = new
    V153 = new
    V154 = new
    V155 = new
    V156 = new
    V157 = new
    V158 = new
    V159 = new
    V160 = new
    V161 = new
    V162 = new
    V163 = new
    V164 = new
    V165 = new
    V166 = new
    V167 = new
    V168 = new
    V169 = new
    V170 = new
    V171 = new
    V172 = new
    V173 = new
    V174 = new
    V175 = new
    V176 = new
    V177 = new
    V178 = new
    V179 = new
    V180 = new
    V181 = new
    V182 = new
    V183 = new
    V184 = new
    V185 = new
    V186 = new
    V187 = new
    V188 = new
    V189 = new
    V190 = new
    V191 = new
    V192 = new
    V193 = new
    V194 = new
    V195 = new
    V196 = new
    V197 = new
    V198 = new
    V199 = new
    V200 = new
    V201 = new
    V202 = new
    V203 = new
    V204 = new
    V205 = new
    V206 = new
    V207 = new
    V208 = new
    V209 = new
    V210 = new
    V211 = new
    V212 = new
    V213 = new
    V214 = new
    V215 = new
    V216 = new
    V217 = new
    V218 = new
    V219 = new
    V220 = new
    V221 = new
    V222 = new
    V223 = new
    V224 = new
    V225 = new
    V226 = new
    V227 = new
    V228 = new
    V229 = new
    V230 = new
    V231 = new
    V232 = new
    V233 = new
    V234 = new
    V235 = new
    V236 = new
    V237 = new
    V238 = new
    V239 = new
    V240 = new
    V241 = new
    V242 = new
    V243 = new
    V244 = new
    V245 = new
    V246 = new
    V247 = new
    V248 = new
    V249 = new
    V250 = new
    V251 = new
    V252 = new
    V253 = new
    V254 = new
    V255 = new
    V256 = new
    V257 = new
    V258 = new
    V259 = new
    V260 = new
    V261 = new
    V262 = new
    V263 = new
    V264 = new
    V265 = new
    V266 = new
    V267 = new
    V268 = new
    V269 = new
    V270 = new
    V271 = new
    V272 = new
    V273 = new
    V274 = new
    V275 = new
    V276 = new
    V277 = new
    V278 = new
    V279 = new
    V280 = new
    V281 = new
    V282 = new
    V283 = new
    V284 = new
    V285 = new
    V286 = new
    V287 = new
    V288 = new
    V289 = new
    V290 = new
    V291 = new
    V292 = new
    V293 = new
    V294 = new
    V295 = new
    V296 = new
    V297 = new
    V298 = new
    V299 = new
    V300 = new
    V301 = new
    V302 = new
    V303 = new
    V304 = new
    V305 = new
    V306 = new
    V307 = new
    V308 = new
    V309 = new
    V310 = new
    V311 = new
    V312 = new
    V313 = new
    V314 = new
    V315 = new
    V316 = new
    V317 = new
    V318 = new
    V319 = new
    V320 = new
    V321 = new
    V322 = new
    V323 = new
    V324 = new
    V325 = new
    V326 = new
    V327 = new
    V328 = new
    V329 = new
    V330 = new
    V331 = new
    V332 = new
    V333 = new
    V334 = new
    V335 = new
    V336 = new
    V337 = new
    V338 = new
    V339 = new
    V340 = new
    V341 = new
    V342 = new
    V343 = new
    V344 = new
    V345 = new
    V346 = new
    V347 = new
    V348 = new
    V349 = new
    V350 = new
    V351 = new
    V352 = new
    V353 = new
    V354 = new
    V355 = new
    V356 = new
    V357 = new
    V358 = new
    V359 = new
    V360 = new
    V361 = new
    V362 = new
    V363 = new
    V364 = new
    V365 = new
    V366 = new
    V367 = new
    V368 = new
    V369 = new
    V370 = new
    V371 = new
    V372 = new
    V373 = new
    V374 = new
    V375 = new
    V376 = new
    V377 = new
    V378 = new
    V379 = new
    V380 = new
    V381 = new
    V382 = new
    V383 = new
    V384 = new
    V385 = new
    V386 = new
    V387 = new
    V388 = new
    V389 = new
    V390 = new
    V391 = new
    V392 = new
    V393 = new
    V394 = new
    V395 = new
    V396 = new
    V397 = new
    V398 = new
    V399 = new
    V400 = new
    V401 = new
    V402 = new
    V403 = new
    V404 = new
    V405 = new
    V406 = new
    V407 = new
    V408 = new
    V409 = new
    V410 = new
    V411 = new
    V412 = new
    V413 = new
    V414 = new
    V415 = new
    V416 = new
    V417 = new
    V418 = new
    V419 = new
    V420 = new
    V421 = new
    V422 = new
    V423 = new
    V424 = new
    V425 = new
    V426 = new
    V427 = new
    V428 = new
    V429 = new
    V430 = new
    V431 = new
    V432 = new
    V433 = new
    V434 = new
    V435 = new
    V436 = new
    V437 = new
    V438 = new
    V439 = new
    V440 = new
    V441 = new
    V442 = new
    V443 = new
    V444 = new
    V445 = new
    V446 = new
    V447 = new
    V448 = new
    V449 = new
    V450 = new
    V451 = new
    V452 = new
    V453 = new
    V454 = new
    V455 = new
    V456 = new
    V457 = new
    V458 = new
    V459 = new
    V460 = new
    V461 = new
    V462 = new
    V463 = new
    V464 = new
    V465 = new
    V466 = new
    V467 = new
    V468 = new
    V469 = new
    V470 = new
    V471 = new
    V472 = new
    V473 = new
    V474 = new
    V475 = new
    V476 = new
    V477 = new
    V478 = new
    V479 = new
    V480 = new
    V481 = new
    V482 = new
    V483 = new
    V484 = new
    V485 = new
    V486 = new
    V487 = new
    V488 = new
    V489 = new
    V490 = new
    V491 = new
    V492 = new
    V493 = new
    V494 = new
    V495 = new
    V496 = new
    V497 = new
    V498 = new
    V499 = new
    V500 = new
    V501 = new
    V502 = new
    V503 = new
    V504 = new
    V505 = new
    V506 = new
    V507 = new
    V508 = new
    V509 = new
    V510 = new
    V511 = new
  end
end

extend T::Sig

sig {params(purpose: Purpose).returns(Integer)}
def overlapping_groups(purpose)
  group0 = purpose == Purpose::V0 || purpose == Purpose::V1
  group1 = purpose == Purpose::V1 || purpose == Purpose::V2
  group2 = purpose == Purpose::V2 || purpose == Purpose::V3
  group3 = purpose == Purpose::V3 || purpose == Purpose::V4
  group4 = purpose == Purpose::V4 || purpose == Purpose::V5
  group5 = purpose == Purpose::V5 || purpose == Purpose::V6
  group6 = purpose == Purpose::V6 || purpose == Purpose::V7
  group7 = purpose == Purpose::V7 || purpose == Purpose::V8
  group8 = purpose == Purpose::V8 || purpose == Purpose::V9
  group9 = purpose == Purpose::V9 || purpose == Purpose::V10
  group10 = purpose == Purpose::V10 || purpose == Purpose::V11
  group11 = purpose == Purpose::V11 || purpose == Purpose::V12
  group12 = purpose == Purpose::V12 || purpose == Purpose::V13
  group13 = purpose == Purpose::V13 || purpose == Purpose::V14
  group14 = purpose == Purpose::V14 || purpose == Purpose::V15
  group15 = purpose == Purpose::V15 || purpose == Purpose::V16
  group16 = purpose == Purpose::V16 || purpose == Purpose::V17
  group17 = purpose == Purpose::V17 || purpose == Purpose::V18
  group18 = purpose == Purpose::V18 || purpose == Purpose::V19
  group19 = purpose == Purpose::V19 || purpose == Purpose::V20
  group20 = purpose == Purpose::V20 || purpose == Purpose::V21
  group21 = purpose == Purpose::V21 || purpose == Purpose::V22
  group22 = purpose == Purpose::V22 || purpose == Purpose::V23
  group23 = purpose == Purpose::V23 || purpose == Purpose::V24
  result = 0
  if group0
    result += 1
  end
  if group1
    result += 2
  end
  if group2
    result += 3
  end
  if group3
    result += 4
  end
  if group4
    result += 5
  end
  if group5
    result += 6
  end
  if group6
    result += 7
  end
  if group7
    result += 8
  end
  if group8
    result += 9
  end
  if group9
    result += 10
  end
  if group10
    result += 11
  end
  if group11
    result += 12
  end
  if group12
    result += 13
  end
  if group13
    result += 14
  end
  if group14
    result += 15
  end
  if group15
    result += 16
  end
  if group16
    result += 17
  end
  if group17
    result += 18
  end
  if group18
    result += 19
  end
  if group19
    result += 20
  end
  if group20
    result += 21
  end
  if group21
    result += 22
  end
  if group22
    result += 23
  end
  if group23
    result += 24
  end
  result
end
