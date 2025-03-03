log_info("akhenaten: figures info started")

figure_fireman = {
	animations : {
		walk : { pos : [0, 0], anim_id: IMG_FIREMAN },
		death : { pos : [0, 0], anim_id: IMG_FIREMAN_DEATH },
    fight_fire : { pos : [0, 0], anim_id: IMG_FIREMAN_FIGHT_FIRE, max_frames: 36 }
  },
  sounds : {
  	fighting_fire : {sound:"fireman_e01.wav", group: 242, text:0},
 		going_to_fire : {sound:"fireman_e02.wav", group: 242, text:1},
 		fighting_fire_also : {sound:"fireman_e03.WAV", group: 242, text:2},
 		desease_can_start_at_any_moment : {sound:"fireman_g01.WAV", group: 242, text:3},
 		no_food_in_city : {sound:"fireman_g02.WAV", group: 242, text:4},
 		city_not_safety_workers_leaving : {sound:"fireman_g03.WAV", group: 242, text:5},
 		need_workers : {sound:"fireman_g04.WAV", group: 242, text:6},
 		hight_fire_level : {sound:"fireman_g05.WAV", group: 242, text:7},
 		gods_are_angry : {sound:"fireman_g06.WAV", group: 242, text:8},
 		need_more_workers : {sound:"fireman_g07.WAV", group: 242, text:9},
 		low_entertainment : {sound:"fireman_g08.WAV", group: 242, text:10},
 		gods_are_pleasures : {sound:"fireman_g09.WAV", group: 242, text:11},
 		city_is_amazing : {sound:"fireman_g10.wav", group: 242, text:12},
  }
}

figure_water_carrier = {
	animations : {
		walk : { pos : [0, 0], anim_id: IMG_WATER_CARRIER },
		death : { pos : [0, 0], anim_id: IMG_WATER_CARRIER_DEATH },
  },
  sounds : {
  	desease_can_start_at_any_moment : {sound: "water_g01.wav", group:237, text:0},
    no_food_in_city : {sound: "water_g02.wav", group:237, text:1},
    city_have_no_army : {sound: "water_g03.wav", group:237, text:2},
    need_workers : {sound: "water_g04.wav", group:237, text:3},
    gods_are_angry : {sound: "water_g05.wav", group:237, text:4},
    city_is_bad : {sound: "water_g06.wav", group:237, text:5},
    much_unemployments : {sound: "water_g07.wav", group:237, text:6},
    low_entertainment : {sound: "water_g08.wav", group:237, text:7},
    city_is_good : {sound: "water_g09.wav", group:237, text:8},
    city_is_amazing : {sound: "water_g10.wav", group:237, text:9},
  }
}

figure_priest = {
	animations : {
		osiris_walk : { pos : [0, 0], anim_id: IMG_PRIEST_OSIRIS, max_frames:12},
		osiris_death : { pos : [0, 0], anim_id: IMG_PRIEST_OSIRIS_DEATH },
		ra_walk : { pos : [0, 0], anim_id: IMG_PRIEST_RA, max_frames:12 },
		ra_death : { pos : [0, 0], anim_id: IMG_PRIEST_RA_DEATH },
		ptah_walk : { pos : [0, 0], anim_id: IMG_PRIEST_PTAH, max_frames:12 },
		ptah_death : { pos : [0, 0], anim_id: IMG_PRIEST_PTAH_DEATH },
		seth_walk : { pos : [0, 0], anim_id: IMG_PRIEST_SETH, max_frames:12 },
		seth_death : { pos : [0, 0], anim_id: IMG_PRIEST_SETH_DEATH },
		bast_walk : { pos : [0, 0], anim_id: IMG_PRIEST_BAST, max_frames:12 },
		bast_death : { pos : [0, 0], anim_id: IMG_PRIEST_BAST_DEATH },
  },
  sounds : {
  	osiris_god_love_festival : {sound:"priest_osiris_e01.wav", group:230, text: 0},
    osiris_city_low_mood : {sound:"priest_osiris_e02.wav", group:230, text: 1},
    osiris_low_entertainment : {sound:"priest_osiris_e03.wav", group:230, text: 2},
    osiris_disease_in_city : {sound:"priest_osiris_e04.wav", group:230, text: 3},
    osiris_city_low_health : {sound:"priest_osiris_g01.wav", group:230, text: 4},
    osiris_no_food_in_city : {sound:"priest_osiris_g02.wav", group:230, text: 5},
    osiris_city_not_safety : {sound:"priest_osiris_g03.wav", group:230, text: 6},
    osiris_need_workers : {sound:"priest_osiris_g04.wav", group:230, text: 7},
    osiris_gods_are_angry : {sound:"priest_osiris_g05.wav", group:230, text: 8},
    osiris_low_sentiment : {sound:"priest_osiris_g06.wav", group:230, text: 9},
    osiris_much_unemployments : {sound:"priest_osiris_g07.wav", group:230, text: 10},
    osiris_need_entertainment : {sound:"priest_osiris_g08.wav", group:230, text: 11},
    osiris_city_is_good : {sound:"priest_osiris_g09.wav", group:230, text: 12},
    osiris_city_is_amazing : {sound:"priest_osiris_g10.wav", group:230, text: 13},

    ra_god_love_festival : {sound:"priest_ra_e01.wav", group:231, text:0},
    ra_city_low_mood : {sound:"priest_ra_e02.wav", group:231, text:1},
    ra_low_entertainment : {sound:"priest_ra_e03.wav", group:231, text:2},
    ra_disease_in_city : {sound:"priest_ra_e04.wav", group:231, text:3},
    ra_city_low_health : {sound:"priest_ra_g01.wav", group:231, text:4},
    ra_no_food_in_city : {sound:"priest_ra_g02.wav", group:231, text:5},
    ra_city_not_safety : {sound:"priest_ra_g03.wav", group:231, text:6},
    ra_need_workers : {sound:"priest_ra_g04.wav", group:231, text:7},
    ra_gods_are_angry : {sound:"priest_ra_g05.wav", group:231, text:8},
    ra_low_sentiment : {sound:"priest_ra_g06.wav", group:231, text:9},
    ra_much_unemployments : {sound:"priest_ra_g07.wav", group:231, text:10},
    ra_need_entertainment : {sound:"priest_ra_g08.wav", group:231, text:11},
    ra_city_is_good : {sound:"priest_ra_g09.wav", group:231, text:12},
    ra_city_is_amazing : {sound:"priest_ra_g10.wav", group:231, text:13},

  	ptah_god_love_festival : {sound:"priest_ptah_e01.wav", group:232, text:0},
    ptah_city_low_mood : {sound:"priest_ptah_e02.wav", group:232, text:1},
    ptah_low_entertainment : {sound:"priest_ptah_e03.wav", group:232, text:2},
    ptah_disease_in_city : {sound:"priest_ptah_e04.wav", group:232, text:3},
    ptah_city_low_health : {sound:"priest_ptah_g01.wav", group:232, text:4},
    ptah_no_food_in_city : {sound:"priest_ptah_g02.wav", group:232, text:5},
    ptah_city_not_safety : {sound:"priest_ptah_g03.wav", group:232, text:6},
    ptah_need_workers : {sound:"priest_ptah_g04.wav", group:232, text:7},
    ptah_gods_are_angry : {sound:"priest_ptah_g05.wav", group:232, text:8},
    ptah_low_sentiment : {sound:"priest_ptah_g06.wav", group:232, text:9},
    ptah_much_unemployments : {sound:"priest_ptah_g07.wav", group:232, text:10},
    ptah_need_entertainment : {sound:"priest_ptah_g08.wav", group:232, text:11},
    ptah_city_is_good : {sound:"priest_ptah_g09.wav", group:232, text:12},
    ptah_city_is_amazing : {sound:"priest_ptah_g10.wav", group:232, text:13},

  	seth_god_love_festival : {sound:"priest_seth_e01.wav", group:233, text:0},
    seth_city_low_mood : {sound:"priest_seth_e02.wav", group:233, text:1},
    seth_low_entertainment : {sound:"priest_seth_e03.wav", group:233, text:2},
    seth_disease_in_city : {sound:"priest_seth_e04.wav", group:233, text:3},
    seth_city_low_health : {sound:"priest_seth_g01.wav", group:233, text:4},
    seth_no_food_in_city : {sound:"priest_seth_g02.wav", group:233, text:5},
    seth_city_not_safety : {sound:"priest_seth_g03.wav", group:233, text:6},
    seth_need_workers : {sound:"priest_seth_g04.wav", group:233, text:7},
    seth_gods_are_angry : {sound:"priest_seth_g05.wav", group:233, text:8},
    seth_low_sentiment : {sound:"priest_seth_g06.wav", group:233, text:9},
    seth_much_unemployments : {sound:"priest_seth_g07.wav", group:233, text:10},
    seth_need_entertainment : {sound:"priest_seth_g08.wav", group:233, text:11},
    seth_city_is_good : {sound:"priest_seth_g09.wav", group:233, text:12},
    seth_city_is_amazing : {sound:"priest_seth_g10.wav", group:233, text:13},

  	bast_god_love_festival: {sound:"bast_e01.wav", groud:234, text:0},
    bast_city_low_mood: {sound:"bast_e02.wav", groud:234, text:1},
    bast_low_entertainment: {sound:"bast_e03.wav", groud:234, text:2},
    bast_disease_in_city: {sound:"bast_e04.wav", groud:234, text:3},
    bast_city_low_health: {sound:"bast_g01.wav", groud:234, text:4},
    bast_no_food_in_city: {sound:"bast_g02.wav", groud:234, text:5},
    bast_city_not_safety: {sound:"bast_g03.wav", groud:234, text:6},
    bast_need_workers: {sound:"bast_g04.wav", groud:234, text:7},
    bast_gods_are_angry: {sound:"bast_g05.wav", groud:234, text:8},
    bast_low_sentiment: {sound:"bast_g06.wav", groud:234, text:9},
    bast_much_unemployments: {sound:"bast_g07.wav", groud:234, text:10},
    bast_need_entertainment: {sound:"bast_g08.wav", groud:234, text:11},
    bast_city_is_good: {sound:"bast_g09.wav", groud:234, text:12},
    bast_city_is_amazing: {sound:"bast_g10.wav", groud:234, text:13},
  }
}

figure_ostrich = {
	animations : {
		idle : { pos : [0, 0], anim_id: ANIM_OSTRICH_IDLE, max_frames:8 },
		eating : { pos : [0, 0], anim_id: ANIM_OSTRICH_EATING, max_frames:7 },
		walk : { pos : [0, 0], anim_id: ANIM_OSTRICH_WALK, max_frames:12 },
		death : { pos : [0, 0], anim_id: ANIM_OSTRICH_DEATH, max_frames:8 },
	}
}

figure_immigrant = {
	animations : {
		swim : { pos : [0, 0], anim_id: ANIM_FERRY_BOAT, max_frames:4, duration:4 },
	},
	sounds : {
		i_need_here: {sound:"immigrant_e01.wav", group: 202, text:0},
 		work_for_all : {sound:"immigrant_e02.wav", group: 202, text:1},
 		cheap_food : {sound:"immigrant_e03.wav", group: 202, text:2},
	}
}

figure_worker = {
	animations : {
		work : { pos : [0, 0], anim_id: IMG_WORKER_AKNH_WORK, max_frames:12 },
	}
}

figure_physician = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_DOCTOR_WALK, max_frames:12 },
		death : { pos : [0, 0], anim_id: ANIM_DOCTOR_DEATH, max_frames:8 },
	},
	sounds : {
		desease_can_start_at_any_moment : {sound:"doctor_e01.wav", group: 239, text:1},
    city_has_low_health : {sound:"doctor_g01.wav", group: 239, text:1},
    no_food_in_city : {sound:"doctor_g02.wav", group: 239, text:2},
    city_have_no_army : {sound:"doctor_g03.wav", group: 239, text:3},
    need_workers : {sound:"doctor_g04.wav", group: 239, text:4},
    gods_are_angry : {sound:"doctor_g05.wav", group: 239, text:5},
    gods_are_pleasures : {sound:"doctor_g06.wav", group: 239, text:6},
    no_job_in_city : {sound:"doctor_g07.wav", group: 239, text:7},
    low_entertainment : {sound:"doctor_g08.wav", group: 239, text:8},
    all_good_in_city : {sound:"doctor_g09.wav", group: 239, text:9},
    city_very_healthy : {sound:"doctor_g10.wav", group: 239, text:10},
	}
}

figure_architect = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_ARCHITECT_WALK, max_frames:12 },
		death : { pos : [0, 0], anim_id: ANIM_ARCHITECT_DEATH, max_frames:8 },
	},
	sounds : {
		extreme_damage_level: {sound:"engineer_e01.wav", group: 243, text:0},
    i_am_works: {sound:"engineer_e02.wav", group: 243, text:1},
    high_damage_level: {sound:"engineer_g01.wav", group: 243, text:2},
    no_food_in_city: {sound:"engineer_g02.wav", group: 243, text:3},
    city_not_safety: {sound:"engineer_g03.wav", group: 243, text:4},
    need_more_workers: {sound:"engineer_g04.wav", group: 243, text:5},
    gods_are_angry: {sound:"engineer_g05.wav", group: 243, text:6},
    city_has_bad_reputation: {sound:"engineer_g06.wav", group: 243, text:7},
    city_is_good: {sound:"engineer_g07.wav", group: 243, text:7},
    low_entertainment: {sound:"engineer_g08.wav", group: 243, text:8},
    city_is_bad: {sound:"engineer_g09.wav", group: 243, text:9},
    city_is_amazing: {sound:"engineer_g10.wav", group: 243, text:10},
	}
}

figure_market_buyer = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_MARKET_BUYER_WALK, max_frames:12 },
	},
	sounds : {
		goto_store : {sound:"mkt_buyer_e01.wav", group:244, text:0 },
    back_to_market : {sound:"mkt_buyer_e02.wav", group:244, text:0 },
    city_has_low_health : {sound:"mkt_buyer_g01.wav", group:244, text:0 },
    no_food_in_city : {sound:"mkt_buyer_g02.wav", group:244, text:0 },
    city_have_no_army : {sound:"mkt_buyer_g03.wav", group:244, text:0 },
    much_unemployments : {sound:"mkt_buyer_g04.wav", group:244, text:0 },
    gods_are_angry : {sound:"mkt_buyer_g05.wav", group:244, text:0 },
    city_is_bad_reputation : {sound:"mkt_buyer_g06.wav", group:244, text:0 },
    too_much_unemployments : {sound:"mkt_buyer_g07.wav", group:244, text:0 },
    low_entertainment : {sound:"mkt_buyer_g08.wav", group:244, text:0 },
    city_is_good : {sound:"mkt_buyer_g09.wav", group:244, text:0 },
    city_is_amazing : {sound:"mkt_buyer_g10.wav", group:244, text:0 },
	}
}

figure_delivery_boy = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_DELIVERY_BOY_WALK, max_frames:12 },
		death : { pos : [0, 0], anim_id: ANIM_DELIVERY_BOY_DEATH, max_frames:8 },
	},
	sounds : {
		those_baskets_too_heavy : {sound: "marketboy_e01.wav", group:221, text: 0},
    i_works_all_day : {sound: "marketboy_e02.wav", group:221, text: 1},
    upon_ill_be_market_owner : {sound: "marketboy_e03.wav", group:221, text: 2},
	}
}

figure_cartpusher = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_CARTPUSHER_WALK, max_frames:12 },
	}
}

figure_storageyard_cart = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_CARTPUSHER_WALK, max_frames:12 },
	}
}

figure_trade_ship = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_TRADER_SHIP_WALK, max_frames:3 },
		idle : { pos : [0, 0], anim_id: ANIM_TRADER_SHIP_IDLE, max_frames:1 },
	}
}

figure_emigrant = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_EMIGRANT_WALK, max_frames:12 },
		death : { pos : [0, 0], anim_id: ANIM_EMIGRANT_DEATH, max_frames:8 },
	},
	sounds : {
		no_job_in_city : {sound:"emigrant_e01.wav", group: 203, text:0},
    no_food_in_city : {sound:"emigrant_e02.wav", group: 203, text:1},
    tax_too_high : {sound:"emigrant_e03.wav", group: 203, text:2},
    salary_too_low : {sound:"emigrant_e04.wav", group: 203, text:3},
    no_house_for_me : {sound:"emigrant_e05.wav", group: 203, text:4},
	}
}

figure_sled = {
	animations : {
		walk : { pos : [0, 0], anim_id: IMG_BIGSLED_EMPTY, max_frames:1 },
	}
}

figure_herbalist = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_HERBALIST_WALK, max_frames:12 },
		death : { pos : [0, 0], anim_id: ANIM_HERBALIST_DEATH, max_frames:8 },
	},
	sounds : {
		no_threat_malaria_here : {sound:"apothecary_e02.wav", group: 241, text:1},
    have_malaria_risk_here : {sound:"apothecary_e03.wav", group: 241, text:2},
	}
}

figure_dancer = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_DANCER_WALK, max_frames:12 },
		death : { pos : [0, 0], anim_id: ANIM_DANCER_DEATH, max_frames:8 },
	},
	sounds : {
		i_like_festivals : {sound:"dancer_e01.wav", group:228, text:0},
    desease_can_start_at_any_moment : {sound:"dancer_g01.wav", group:228, text:1},
    no_food_in_city : {sound:"dancer_g02.wav", group:228, text:2},
    city_not_safety_workers_leaving : {sound:"dancer_g03.wav", group:228, text:3},
    need_workers : {sound:"dancer_g04.wav", group:228, text:4},
    gods_are_angry : {sound:"dancer_g05.wav", group:228, text:5},
    city_is_bad : {sound:"dancer_g06.wav", group:228, text:6},
    much_unemployments : {sound:"dancer_g07.wav", group:228, text:7},
    salary_too_low : {sound:"dancer_g08.wav", group:228, text:8},
    city_is_good : {sound:"dancer_g09.wav", group:228, text:9},
    city_is_amazing : {sound:"dancer_g10.wav", group:228, text:10},
	}
}

figure_musician = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_MUSICIAN_WALK, max_frames:12 },
		death : { pos : [0, 0], anim_id: ANIM_MUSICIAN_DEATH, max_frames:8 },
	},
	sounds : {
		i_like_festivals : { sound:"musician_e01.wav", group:229, text:0 },
 		city_heath_too_low : { sound:"musician_g01.wav", group:229, text:0 },
 		no_food_in_city : { sound:"musician_g02.wav", group:229, text:0 },
 		city_not_safety_workers_leaving : { sound:"musician_g03.wav", group:229, text:0 },
 		need_workers : { sound:"musician_g04.wav", group:229, text:0 },
 		gods_are_angry : { sound:"musician_g05.wav", group:229, text:0 },
 		city_is_bad_reputation : { sound:"musician_g06.wav", group:229, text:0 },
 		much_unemployments : { sound:"musician_g07.wav", group:229, text:0 },
 		no_entertainment_need : { sound:"musician_g08.wav", group:229, text:0 },
 		city_not_bad : { sound:"musician_g09.wav", group:229, text:0 },
 		city_is_good : { sound:"musician_g10.wav", group:229, text:0 },
	}
}

figure_juggler = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_JUGGLER_WALK, max_frames:12 },
		death : { pos : [0, 0], anim_id: ANIM_JUGGLER_DEATH, max_frames:8 },
	},
	sounds : {
		i_like_festivals : {sound:"juggler_e01.wav", group: 230, text:0 },
    disease_in_city : {sound:"juggler_g01.wav", group: 230, text:0 },
    city_low_sentiment : {sound:"juggler_g02.wav", group: 230, text:0 },
    city_not_safety_workers_leaving : {sound:"juggler_g03.wav", group: 230, text:0 },
    salary_too_low : {sound:"juggler_g04.wav", group: 230, text:0 },
    gods_are_angry : {sound:"juggler_g05.wav", group: 230, text:0 },
    city_verylow_sentiment : {sound:"juggler_g06.wav", group: 230, text:0 },
    much_unemployments : {sound:"juggler_g07.wav", group: 230, text:0 },
    low_entertainment : {sound:"juggler_g08.wav", group: 230, text:0 },
    city_is_good : {sound:"juggler_g09.wav", group: 230, text:0 },
    city_is_amazing : {sound:"juggler_g10.wav", group: 230, text:0 },
	}
}

figure_market_trader = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_HERBALIST_WALK, max_frames:12 },
		death : { pos : [0, 0], anim_id: ANIM_HERBALIST_DEATH, max_frames:8 },
	},
	sounds : {
		goods_are_finished : {sound:"mkt_seller_e01.wav", group:245, text:0},
 	  we_are_selling_goods : {sound:"mkt_seller_e02.wav", group:245, text:0},
	}
}

figure_labor_seeker = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_LABOR_SEEKER_WALK, max_frames:12 },
		death : { pos : [0, 0], anim_id: ANIM_LABOR_SEEKER_DEATH, max_frames:8 },
	},
	sounds : {
		no_jobs : { sound:"Labor_e01.wav", group:213, text:0},
    need_workers : { sound:"Labor_e02.WAV", group:213, text:1},
    disease_in_city : { sound:"Labor_g01.WAV", group:213, text:2},
    i_looking_for_the_workers : { sound:"Labor_g02.WAV", group:213, text:3},
    city_not_safety_workers_leaving : { sound:"Labor_g03.WAV", group:213, text:4},
    need_more_workers : { sound:"Labor_g04.WAV", group:213, text:5},
    gods_are_angry : { sound:"Labor_g05.WAV", group:213, text:6},
    no_food_in_city : { sound:"Labor_g06.WAV", group:213, text:7},
    much_unemployments : { sound:"Labor_g07.WAV", group:213, text:8},
    no_some_workers : { sound:"Labor_g08.WAV", group:213, text:9},
    i_want_to_leave_city : { sound:"Labor_g09.WAV", group:213, text:10},
    city_is_amazing : { sound:"Labor_g10.WAV", group:213, text:11},
	}
}

figure_reed_gatherer = {
	animations : {
		walk : { pos : [0, 0], anim_id: ANIM_REED_GATHERER_WALK, max_frames:12 },
		work : { pos : [0, 0], anim_id: ANIM_REED_GATHERER_WORK, max_frames:15 },
		return : { pos : [0, 0], anim_id: ANIM_REED_GATHERER_RETURN, max_frames:12 },
		death : { pos : [0, 0], anim_id: ANIM_REED_GATHERER_DEATH, max_frames:8 },
	},
}

figure_explosion = {
	animations : {
		poof : { pos : [0, 0], anim_id: IMG_EXPLOSION, max_frames:12 },
	}
}

figure_tax_collector = {
	animations : {
		walk : { pos : [0, 0], anim_id: IMG_BIGSLED_EMPTY, max_frames:1 },
	},
	sounds : {
		need_more_tax_collectors: {sound: "taxman_e01.wav", group: 245, text:0},
    high_taxes: {sound: "taxman_e02.wav", group: 245, text:1},
    much_pooh_houses: {sound: "taxman_e03.wav", group: 245, text:2},
    desease_can_start_at_any_moment: {sound: "taxman_g01.wav", group: 245, text:3},
    no_food_in_city: {sound: "taxman_g02.wav", group: 245, text:4},
    city_have_no_army: {sound: "taxman_g03.wav", group: 245, text:5},
    need_workers: {sound: "taxman_g04.wav", group: 245, text:6},
    gods_are_angry: {sound: "taxman_g05.wav", group: 245, text:7},
    city_is_bad: {sound: "taxman_g06.wav", group: 245, text:8},
    much_unemployments: {sound: "taxman_g07.wav", group: 245, text:9},
    low_entertainment: {sound: "taxman_g08.wav", group: 245, text:10},
    city_is_good: {sound: "taxman_g09.wav", group: 245, text:11},
    city_is_amazing: {sound: "taxman_g10.wav", group: 245, text:12},
	}
}
