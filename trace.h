static inline void record_phase(int ) {}
static inline void record_scale_factor(int ) {}

static inline void record_piece_value(Color , Piece_type ) {}

static inline void record_pawn_psqt(  Color , unsigned ) {}
static inline void record_knight_psqt(Color , unsigned ) {}
static inline void record_bishop_psqt(Color , unsigned ) {}
static inline void record_rook_psqt(  Color , unsigned ) {}
static inline void record_queen_psqt( Color , unsigned ) {}
static inline void record_king_psqt(  Color , unsigned ) {}

static inline void record_knight_mobility(Color , unsigned) {}
static inline void record_bishop_mobility(Color , unsigned) {}
static inline void record_rook_mobility(Color , unsigned) {}
static inline void record_queen_mobility(Color , unsigned) {}
static inline void record_king_mobility(Color , unsigned) {}

static inline void record_isolated_pawn(Color ) {}
static inline void record_doubled_pawn(Color ) {}
static inline void record_backward_pawn(Color , unsigned ) {}
static inline void record_backward_pawn_half_open(Color , unsigned ) {}
static inline void record_chained_pawn(Color , unsigned ) {}
static inline void record_phalanx_pawn(Color , unsigned ) {}

static inline void record_passed_pawn(Color , unsigned ) {}
static inline void record_passed_pawn_blocked(Color , unsigned) {}
static inline void record_passed_pawn_safe_advance(Color ) {}
static inline void record_passed_pawn_safe_path(Color ) {}
static inline void record_passed_friendly_distance(Color, unsigned , unsigned ) {}
static inline void record_passed_enemy_distance(Color, unsigned , unsigned ) {}

static inline void record_knight_outpost(Color ) {}
static inline void record_knight_outpost_supported(Color ) {}

static inline void record_bishop_pawn(Color , unsigned ) {}
static inline void record_double_bishop(Color ) {}

static inline void record_rook_open_file(Color ) {}
static inline void record_rook_half_open_file(Color ) {}
static inline void record_rook_on_seventh(Color ) {}

static inline void record_pawn_shelter(Color, bool, unsigned, unsigned ) {}
static inline void record_pawn_storm(Color, bool, unsigned, unsigned ) {}

static inline void record_king_attacker_weight(Color , Piece_type ) {}
static inline void record_adjust_king_attacker_weights(Color , unsigned ) {}
static inline void record_clear_attacker_weights(Color ) {}
static inline void record_king_zone_attack_count_weight(Color , unsigned ) {}
static inline void record_king_zone_weak_square(Color , unsigned) {}
static inline void record_safe_knight_check(Color , unsigned ) {}
static inline void record_safe_bishop_check(Color , unsigned ) {}
static inline void record_safe_rook_check(  Color , unsigned ) {}
static inline void record_safe_queen_check( Color , unsigned ) {}
static inline void record_unsafe_check(Color , unsigned ) {}
static inline void record_king_danger_no_queen_weight(Color , bool ) {}
static inline void record_king_danger_offset(Color ) {}

static inline void record_center_control(Color, unsigned ) {}

static inline void record_minor_threatened_by_pawn(  Color, unsigned ) {}
static inline void record_minor_threatened_by_minor( Color, unsigned ) {}
static inline void record_rook_threatened_by_lesser( Color, unsigned ) {}
static inline void record_queen_threatened_by_lesser(Color, unsigned ) {}
static inline void record_minor_threatened_by_major( Color, unsigned ) {}
