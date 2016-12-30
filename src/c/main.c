#define SETTINGS_KEY 1
#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer, *s_weekday_layer, *s_degrees_layer, *s_date_layer, *s_conditions_layer, *s_weather_layer, *s_city_layer;
static GFont s_time_font, s_date_font, s_conditions_font;
const char * const DOZ_GROUPS[] = {"0","1","2","3","4","5","6","7","8","9","X","E"};
const char DOZ_DIGITS[] = { '0','1','2','3','4','5','6','7','8','9','X','E' };
const int ms_clock_offset = 1000;
char stale_uv = 0;

const int WEATHER_REFRESH_MINS = 10;
const int chalk_degrees_x=25, default_degrees_x=6, chalk_date_y=15, default_date_y=10, chalk_weekday_y=67, default_weekday_y=62, chalk_degrees_y=67, default_degrees_y=62, default_time_y=24, default_weather_y=80, default_conditions_y=98, default_city_y1=134, default_city_y2=116, chalk_time_y=29, chalk_weather_y=85, chalk_conditions_y=103, chalk_city_y1=139, chalk_city_y2=121;
static char is_chalk = PBL_IF_ROUND_ELSE(1,0);

typedef struct ClaySettings {
  int diurnal, temperature_scale, wind_speed_format, uv_format, date_format, pressure_mb, pressure_format, wind_degrees, wind_degrees_format, origin, hemisphere, season;
 	float temperature, wind_kph, uv, humidity, latitude;
 	char city[28], conditions[42], weekday_letter[8], api_key[20];
} __attribute__((__packed__)) ClaySettings;

typedef struct month_struct {
	int year,month,day;
} month_struct;

ClaySettings settings;

static char SPRING_A1[12] = {31,31,31,31,31,31,30,30,30,30,30,30};
static char SPRING_B1[12] = {31,30,31,31,31,31,30,30,30,30,30,30};
static char SPRING_B2[12] = {31,31,31,31,31,30,30,30,30,30,30,30};
static char SUMMER_A1[12] = {31,31,31,31,30,30,30,30,30,30,31,31};
static char SUMMER_B1[12] = {31,31,31,31,30,30,30,30,30,30,30,31};
static char SUMMER_B2[12] = {31,31,31,30,30,30,30,30,30,30,31,31};
static char AUTUMN_A1[12] = {30,30,30,30,30,30,31,31,31,31,31,31}; 
static char AUTUMN_B1[12] = {30,30,30,30,30,30,30,31,31,31,31,31};
static char AUTUMN_B2[12] = {30,30,30,30,30,30,30,31,31,31,31,31};
static char WINTER_A1[12] = {30,30,30,30,31,31,31,31,31,31,30,30};
static char WINTER_B1[12] = {30,30,30,30,30,31,31,31,31,31,30,30};
static char WINTER_B2[12] = {30,30,30,30,31,31,31,31,31,30,30,30};

#define LEN 11

struct Season {
	int year[LEN];
	int start[LEN];
	char *days[LEN];
};

struct Season SPRING = {
	.year = {2016,2017,2018,2019,2020,2021,2022,2023,2024,2025,2026},
	.start = {20160320,20170320,20180320,20190320,20200320,20210320,20220320,20230320,20240320,20250320,20260320},
	.days = {&SPRING_B1[0],&SPRING_B2[0],&SPRING_B2[0],&SPRING_A1[0],&SPRING_B1[0],&SPRING_B2[0],&SPRING_B2[0],&SPRING_A1[0],&SPRING_B1[0],&SPRING_B2[0],&SPRING_B2[0]}
};

struct Season SUMMER = {
	.year = {2016,2017,2018,2019,2020,2021,2022,2023,2024,2025,2026},
	.start = {20160620,20170621,20180621,20190620,20200620,20210621,20220621,20230620,20240620,20250621,20260621},
	.days = {&SUMMER_A1[0],&SUMMER_B2[0],&SUMMER_B2[0],&SUMMER_B1[0],&SUMMER_A1[0],&SUMMER_B2[0],&SUMMER_B2[0],&SUMMER_B1[0],&SUMMER_A1[0],&SUMMER_B2[0],&SUMMER_B2[0]}
};

struct Season AUTUMN = {
	.year = {2016,2017,2018,2019,2020,2021,2022,2023,2024,2025,2026},
	.start = {20160922,20170922,20180923,20190923,20200922,20210922,20220923,20230923,20240922,20250922,20260923},
	.days = {&AUTUMN_B1[0],&AUTUMN_A1[0],&AUTUMN_B2[0],&AUTUMN_B2[0],&AUTUMN_B1[0],&AUTUMN_A1[0],&AUTUMN_B2[0],&AUTUMN_B2[0],&AUTUMN_B1[0],&AUTUMN_A1[0],&AUTUMN_B2[0]}
};

struct Season WINTER = {
	.year = {2016,2017,2018,2019,2020,2021,2022,2023,2024,2025,2026},
	.start = {20151222,20161221,20171221,20181221,20191222,20201221,20211221,20221221,20231222,20241221,20251221},
	.days = {&WINTER_B1[0],&WINTER_B2[0],&WINTER_B2[0],&WINTER_A1[0],&WINTER_B1[0],&WINTER_B2[0],&WINTER_B2[0],&WINTER_A1[0],&WINTER_B1[0],&WINTER_B2[0],&WINTER_B2[0]}
};

int time_from_yyyymmdd(int yyyymmdd){

  int year = yyyymmdd/10000;
  int month = (yyyymmdd%10000)/100;
	int day = yyyymmdd%100;

	return mktime(&(struct tm){.tm_year=year-1900,.tm_mon=month-1,.tm_mday=day});
	
}

month_struct get_dozenal_date(struct Season *this, int query){
	
	int adjust, seconds = query-time_from_yyyymmdd(this->start[0])+86400;

	if(seconds<=0){ //APP_LOG(0,"underflow calendar");
   return (month_struct) {.year=2000,.month=1,.day=1}; }
	
	for(int year=0;year<LEN;++year){
  	for(int month=1;month<=12;++month){
    	adjust = 86400*(*(this->days+year))[month-1];
      if(adjust<seconds){ seconds = seconds-adjust; }
      else{ return (month_struct) {.year=year+this->year[0],.month=month,.day=seconds/86400}; }
     }
	}

  //APP_LOG(0,"overflow calendar");
  return (month_struct) {.year=2020,.month=1,.day=1};

}

int get_origin_offset(){
	if(settings.origin==1){ return(-21600); } // shift time back by six hours
	else if(settings.origin==2){ return(-28800); } // shift time back by eight hours
  else if(settings.origin==3){ return(-43200); } // shift time back by twelve hours
  else if(settings.origin==4){ return(-14400); } // shift time back by four hours
	else { return(0); }
}

char *replace_str(char *str,char *orig,char *rep)
{

	char *p;
  static char buffer[50];
	if(!(p = strstr(str,orig))) { strcpy(buffer,str); return buffer; }	
	strncpy(buffer,str,p-str);
	buffer[p-str]='\0';
	strcat(buffer,rep);
	if(p-str+strlen(orig)<strlen(str)){ strcat(buffer,p+strlen(orig)); }
	return buffer;

}

static int ms_to_next_lull(){

	int ms_wait;
	time_t now = time(NULL)+get_origin_offset();
	struct tm *tick_time = localtime(&now);
	ms_wait = 4000*tick_time->tm_min + 2400*tick_time->tm_sec + 2.4*(time_ms(NULL,NULL)+ms_clock_offset);
	ms_wait = 10000 - (ms_wait % 10000);
	ms_wait = 25*ms_wait/60;
	return(ms_wait);
	
}

static void write_dozenal(int input, int required_length, char * buffer){
	
	char output[20], is_negative;
	const char doz[13] = {'0','1','2','3','4','5','6','7','8','9','X','E'};
	int y = sizeof(output)-1;
	
	// terminate string with null character
	
	output[y] = '\0';
	
	if(input < 0){ is_negative = 1; input = -input; } else { is_negative = 0; }
	
	// divide by 12 to digest a dozenal digit
	// continue until number fully digested or leading zeros set as needed
	
	while((input > 0) || (required_length > 0)){
		y--;
		output[y] = doz[input % 12];
		input = input/12;	
		required_length--;
	}
	
	// include negative sign if needed
	
	if (is_negative == 1){ 
		y--;
		output[y] = '-';
	}
	
	// save into buffer passed by calling function
		
	strcpy(buffer, output+y);
	
}

static void request_weather(void) {
	
	// request data from phone via app.js

	char *ptr = settings.api_key;	
	
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);  
	if (!iter) { return; }
	
	dict_write_cstring(iter,MESSAGE_KEY_API_KEY,ptr);
	dict_write_end(iter);
  app_message_outbox_send();

}

static void refresh_weather(){
	
	// update weather display according to format set in config.js

	static char weather_buffer[20], weekday_buffer[20], degrees_buffer[20];
  char humidity_buffer[8], temperature_buffer[8], wind_kph_buffer[8], uv_buffer[8], pressure_mb_buffer[4], wind_degrees_buffer[4];
  int temperature, humidity, wind_kph, uv, pressure_mb, wind_degrees, alignment, degrees_y;
	GSize g_conditions_layer, g_city_layer, g_degrees_layer, g_weekday_layer;
	GRect r_degrees_layer, r_weekday_layer;
	
	// add 0.5 to ensure proper rounding from float to integer
	
	switch(settings.temperature_scale){
		case 4: temperature = (settings.temperature-32)/1.8+0.5; break; //°C
		case 5: temperature = settings.temperature+0.5; break; //°F
		case 0: temperature = 1.4*(settings.temperature-32)+0.5; break; //stadigrees crystallic
    case 3: temperature = 0.8*(settings.temperature-32)+0.5; break; //°G
		case 2: temperature = 0.463*(settings.temperature-32)+0.5; break; //tregrees
    default: temperature = 1.4*(settings.temperature-32)+48.5; break; //stadigrees familiar
	}
	
	switch(settings.wind_speed_format){
		case 0: wind_kph = 0.9799*settings.wind_kph+0.5; break; //velocitels
		case 2: wind_kph = settings.wind_kph+0.5; break; //km/h
    case 1: wind_kph = 1.9572*settings.wind_kph+0.5; break; //unciaVlos
		default: wind_kph = 0.6214*settings.wind_kph+0.5; break; // mph
	}
  
  if(settings.uv>=0){
    switch(settings.uv_format){
	    case 0: uv = 0.9104*settings.uv+0.5; break; //quadciaintensitels
	    case 1: uv = 0.6044*settings.uv+0.5; break; //hexciaPenz
	    default : uv = settings.uv+0.5; break; //SI intensity
    }
    write_dozenal(uv,1,uv_buffer);
    if(stale_uv==1){ strcat(uv_buffer,"'"); }
  } else { strcpy(uv_buffer,"--"); }

	switch(settings.pressure_format){
		case 0: pressure_mb = settings.pressure_mb*1.2446+0.5; break; //pressurels
    case 2: pressure_mb = settings.pressure_mb*0.4137+0.5; break; //unciaPrems
    case 3: pressure_mb = settings.pressure_mb*0.3653+0.5; break; //biciaGrafuts Hg
		default: pressure_mb = settings.pressure_mb*1.0974+0.5; break; //uncialengthels Hg
    case 4: pressure_mb = settings.pressure_mb+0.5; break; //millibars
    //case 0: pressure_mb = settings.pressure_mb*42.1457+0.5; break; //pressurels from inches
    //case 2: pressure_mb = settings.pressure_mb*14.0098+0.5; break; //unciaPrems from inches
    //case 3: pressure_mb = settings.pressure_mb*12.37+0.5; break; //biciaGrafuts Hg from inches
    //default: pressure_mb = settings.pressure_mb*37.1613+0.5; break; //uncialengthels Hg from inches
	}
  
	if ((settings.humidity>0)&(settings.humidity<=100)){
		humidity = 1.44*settings.humidity+0.5;	
		write_dozenal(humidity,1,humidity_buffer);
	} else { snprintf(humidity_buffer,sizeof(humidity_buffer),"--"); }
	
	if ((settings.wind_degrees>=0)&(settings.wind_degrees<=360)){
    switch(settings.wind_degrees_format){
      case 0: wind_degrees = 0.4*(settings.wind_degrees%360)+0.5;	break; //biciaturns
      case 2: wind_degrees = settings.wind_degrees%360+0.5;	break; //degrees
		  default: wind_degrees = 0.0667*(settings.wind_degrees%360)+0.5;	break; //unciaPis
    }
    write_dozenal(wind_degrees,1,wind_degrees_buffer);
	} else { strcpy(wind_degrees_buffer,"--"); }
	
	write_dozenal(temperature,1,temperature_buffer);
	write_dozenal(wind_kph,1,wind_kph_buffer);
	write_dozenal(pressure_mb,1,pressure_mb_buffer);

	//snprintf(weekday_buffer, sizeof(weekday_buffer), "%s %s° %s %s", settings.weekday_letter, temperature_buffer, pressure_mb_buffer, uv_buffer);
  //splitting weekday layer into weekday and degrees:
  snprintf(weekday_buffer, sizeof(weekday_buffer), "%s", settings.weekday_letter);
  text_layer_set_text(s_weekday_layer, weekday_buffer);
  
  snprintf(degrees_buffer, sizeof(degrees_buffer), "%s° %s %s", temperature_buffer, pressure_mb_buffer, uv_buffer);
  text_layer_set_text(s_degrees_layer, degrees_buffer);
  
	snprintf(weather_buffer, sizeof(weather_buffer), "%s%% %s·%s", humidity_buffer, wind_kph_buffer, wind_degrees_buffer);
	text_layer_set_text(s_weather_layer, weather_buffer);
 	
	//if (strlen(settings.api_key)<10) { strcpy(settings.conditions,"Please load API key"); }
  text_layer_set_text(s_conditions_layer, settings.conditions);
	text_layer_set_text(s_city_layer,settings.city);
	
	g_conditions_layer = text_layer_get_content_size(s_conditions_layer);
	g_city_layer = text_layer_get_content_size(s_city_layer);
		
	if(g_city_layer.h<25){
    
		degrees_y =  PBL_IF_ROUND_ELSE(chalk_degrees_y,default_degrees_y);
    layer_set_frame(text_layer_get_layer(s_date_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_date_y,default_date_y),PBL_IF_ROUND_ELSE(180,144),22));
		layer_set_frame(text_layer_get_layer(s_time_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_time_y, default_time_y),PBL_IF_ROUND_ELSE(180,144),40));
		layer_set_frame(text_layer_get_layer(s_weather_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_weather_y,default_weather_y),PBL_IF_ROUND_ELSE(180,144),22));
		layer_set_frame(text_layer_get_layer(s_conditions_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_conditions_y,default_conditions_y),PBL_IF_ROUND_ELSE(180,144),66));
		layer_set_frame(text_layer_get_layer(s_city_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_city_y1,default_city_y1),PBL_IF_ROUND_ELSE(180,144),44));
		layer_set_frame(text_layer_get_layer(s_weekday_layer),GRect(PBL_IF_ROUND_ELSE(chalk_degrees_x,default_degrees_x),degrees_y,PBL_IF_ROUND_ELSE(180-2*chalk_degrees_x,144-2*default_degrees_x),22));
    layer_set_frame(text_layer_get_layer(s_degrees_layer),GRect(PBL_IF_ROUND_ELSE(chalk_degrees_x,default_degrees_x),degrees_y,PBL_IF_ROUND_ELSE(180-2*chalk_degrees_x,144-2*default_degrees_x),22));
  
  }
	else if((g_conditions_layer.h<25)&(g_city_layer.h>25)){

		degrees_y = PBL_IF_ROUND_ELSE(chalk_weekday_y,default_weekday_y);
    layer_set_frame(text_layer_get_layer(s_date_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_date_y,default_date_y),PBL_IF_ROUND_ELSE(180,144),22));
    layer_set_frame(text_layer_get_layer(s_time_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_time_y,default_time_y),PBL_IF_ROUND_ELSE(180,144), 40));
		layer_set_frame(text_layer_get_layer(s_weather_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_weather_y,default_weather_y),PBL_IF_ROUND_ELSE(180,144),22));
		layer_set_frame(text_layer_get_layer(s_conditions_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_conditions_y,default_conditions_y),PBL_IF_ROUND_ELSE(180,144),66));
		layer_set_frame(text_layer_get_layer(s_city_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_city_y2,default_city_y2),PBL_IF_ROUND_ELSE(180,144),44));
	  layer_set_frame(text_layer_get_layer(s_weekday_layer),GRect(PBL_IF_ROUND_ELSE(chalk_degrees_x,default_degrees_x),degrees_y,PBL_IF_ROUND_ELSE(180-2*chalk_degrees_x,144-2*default_degrees_x),22));
    layer_set_frame(text_layer_get_layer(s_degrees_layer),GRect(PBL_IF_ROUND_ELSE(chalk_degrees_x,default_degrees_x),degrees_y,PBL_IF_ROUND_ELSE(180-2*chalk_degrees_x,144-2*default_degrees_x),22));

  }
	else{
	
		degrees_y = PBL_IF_ROUND_ELSE(chalk_weekday_y-10,default_weekday_y-10);
		layer_set_frame(text_layer_get_layer(s_date_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_date_y-10,default_date_y-10),PBL_IF_ROUND_ELSE(180,144),22));
    layer_set_frame(text_layer_get_layer(s_time_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_time_y-10,default_time_y-10),PBL_IF_ROUND_ELSE(180,144),40));
		layer_set_frame(text_layer_get_layer(s_weather_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_weather_y-10,default_weather_y-10),PBL_IF_ROUND_ELSE(180,144),22));
		layer_set_frame(text_layer_get_layer(s_conditions_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_conditions_y-10,default_conditions_y-10),PBL_IF_ROUND_ELSE(180,144),66));
		layer_set_frame(text_layer_get_layer(s_city_layer),GRect(0,PBL_IF_ROUND_ELSE(chalk_city_y1-10,default_city_y1-10),PBL_IF_ROUND_ELSE(180,144),44));
		layer_set_frame(text_layer_get_layer(s_weekday_layer),GRect(PBL_IF_ROUND_ELSE(chalk_degrees_x,default_degrees_x),degrees_y,PBL_IF_ROUND_ELSE(180-2*chalk_degrees_x,144-2*default_degrees_x),22));
    layer_set_frame(text_layer_get_layer(s_degrees_layer),GRect(PBL_IF_ROUND_ELSE(chalk_degrees_x,default_degrees_x),degrees_y,PBL_IF_ROUND_ELSE(180-2*chalk_degrees_x,144-2*default_degrees_x),22));

  }
	
	g_degrees_layer = text_layer_get_content_size(s_degrees_layer);
	g_weekday_layer = text_layer_get_content_size(s_weekday_layer);
	alignment = (PBL_IF_ROUND_ELSE(180,144)-g_degrees_layer.w-g_weekday_layer.w-18)/2;
	if(alignment<2){ alignment = 2; }
	
	r_weekday_layer = layer_get_bounds(text_layer_get_layer(s_weekday_layer));
	r_weekday_layer.origin.x = alignment;
	r_weekday_layer.origin.y = degrees_y;

	r_degrees_layer = layer_get_bounds(text_layer_get_layer(s_degrees_layer));
	r_degrees_layer.origin.x = PBL_IF_ROUND_ELSE(180,144)-g_degrees_layer.w-alignment;
	r_degrees_layer.origin.y = degrees_y;
	r_degrees_layer.size.w = g_degrees_layer.w;

	if(settings.date_format==4){ 
		r_degrees_layer.origin.x = 0;
		r_degrees_layer.size.w = PBL_IF_ROUND_ELSE(180,144);
		text_layer_set_text_alignment(s_degrees_layer, GTextAlignmentCenter);
	} else { text_layer_set_text_alignment(s_degrees_layer, GTextAlignmentLeft); }

	layer_set_frame(text_layer_get_layer(s_weekday_layer),r_weekday_layer);
	layer_set_frame(text_layer_get_layer(s_degrees_layer),r_degrees_layer);
	
}

static void refresh_date() {
	
	// draw date using desired format as set in config.js
	
	const char HOL_DAY_OF_WEEK[] = {'R','A','F','V','C','P'};
	const char *DAY_OF_WEEK[] = {"U","M","T","W","R","F","S"};
	static char date_buffer[20];
	char dozenal_year[5], dozenal_mm[3], dozenal_dd[3], dozenal_m[3], dozenal_d[3], dozenal_buffer[10], month_buffer[8], day_buffer[4];
  time_t now = time(NULL)+get_origin_offset()+1;
	struct tm *tick_time = localtime(&now);
	struct month_struct result;
	
	// override date

  //tick_time->tm_year = 117; // years since 1900
  //tick_time->tm_mon = 9; // 0 to 11
	//tick_time->tm_mday = 22; // 1 to 31
	//tick_time->tm_wday = 2; // Sunday is 0, Monday 1, Tuesday 2, Wednesday 3, Thursday 4, Friday 5, Saturday 6

	// end override
	
  int year = tick_time->tm_year+1900;
  int month = tick_time->tm_mon+1;

	int hemisphere;	
	struct Season *season;
	int query = time_from_yyyymmdd(year*10000+month*100+tick_time->tm_mday);
	
	if(settings.hemisphere==0){ hemisphere = 0; } 
	else if (settings.hemisphere==1){ hemisphere = 1; }
	else if (settings.latitude<0) { hemisphere = 1; }
	else { hemisphere = 0; }

	if(hemisphere==0){
		switch(settings.season){
			case 0: season = &SPRING; break;
			case 1: season = &SUMMER; break;
			case 2: season = &AUTUMN; break;
			default: season = &WINTER; break;
		}
	}else{
		switch(settings.season){
			case 0: season = &AUTUMN; break;
			case 1: season = &WINTER; break;
			case 2: season = &SPRING; break;
			default: season = &SUMMER; break;
		}
	}
	
	if(is_chalk==1){ write_dozenal(year%100,2,dozenal_year); }
	else{ write_dozenal(year,4,dozenal_year); }	
	
	write_dozenal(month,2,dozenal_mm);
	write_dozenal(tick_time->tm_mday,2,dozenal_dd);
	write_dozenal(month,1,dozenal_m);
	write_dozenal(tick_time->tm_mday,1,dozenal_d);

	switch(settings.date_format){	
		
    case 0: 
      snprintf(date_buffer, sizeof(date_buffer), "%s-%s-%s", dozenal_year, dozenal_mm, dozenal_dd); 
      snprintf(settings.weekday_letter, sizeof(settings.weekday_letter), "%s", DAY_OF_WEEK[tick_time->tm_wday]);
      break;
		
    case 1:
      snprintf(date_buffer, sizeof(date_buffer), "%s/%s/%s", dozenal_m, dozenal_d, dozenal_year);
      snprintf(settings.weekday_letter, sizeof(settings.weekday_letter), "%s", DAY_OF_WEEK[tick_time->tm_wday]);
      break;
    
		case 2:
      snprintf(date_buffer, sizeof(date_buffer), "%s.%s.%s", dozenal_d, dozenal_m, dozenal_year);
      snprintf(settings.weekday_letter, sizeof(settings.weekday_letter), "%s", DAY_OF_WEEK[tick_time->tm_wday]);
      break;
		
		case 3:
    case 4:

			result = get_dozenal_date(season,query);
		
			if(result.day==31){ 			
				
				write_dozenal(result.month,1,month_buffer);
				write_dozenal(result.year+9563,4,dozenal_buffer);
				if(is_chalk==1){ strcpy(dozenal_buffer,dozenal_buffer+2); }
				snprintf(date_buffer, sizeof(date_buffer),"%s-S%s",dozenal_buffer,month_buffer);
				settings.weekday_letter[0] = 'S';
				
			}else {
				
				write_dozenal(result.day,2,day_buffer);
				write_dozenal(result.month,2,month_buffer);
				write_dozenal(result.year+9563,4,dozenal_buffer);
				if(is_chalk==1){ strcpy(dozenal_buffer,dozenal_buffer+2); }
				snprintf(date_buffer,sizeof(date_buffer),"%s-%s-%s",dozenal_buffer,month_buffer,day_buffer);
				settings.weekday_letter[0] = HOL_DAY_OF_WEEK[(result.day-1)%6];
				
			}
    
      if(settings.date_format==4){ settings.weekday_letter[0] = '\0'; }
			break;
  
			default:
		
			result = get_dozenal_date(season,query);
      write_dozenal(1728*(result.year+9563)+144*(result.month-1)+12*((result.day-1)/6)+((result.day-1)%6),7,dozenal_buffer);
		
			if(is_chalk==1){ snprintf(date_buffer,sizeof(date_buffer),"%.2s-%.1s-%.1s-%.1s",dozenal_buffer+2,dozenal_buffer+4,dozenal_buffer+5,dozenal_buffer+6); }
			else{ snprintf(date_buffer, sizeof(date_buffer),"%.4s-%.1s-%.1s-%.1s",dozenal_buffer,dozenal_buffer+4,dozenal_buffer+5,dozenal_buffer+6); }	
    
      if(result.day==31){ settings.weekday_letter[0] = 'S'; }
      else { settings.weekday_letter[0] = HOL_DAY_OF_WEEK[(result.day-1)%6]; }
      
			break;
		
	}	
  
  text_layer_set_text(s_date_layer, date_buffer);
	refresh_weather();
	
}

static void refresh_time(){

	static char buffer[8], lead[4], trail[4];
	time_t now = time(NULL)+get_origin_offset();
	struct tm *tick_time= localtime(&now);
	int ensure_change = 50, lull;

	if(settings.diurnal==0){ lull = (int)(864*tick_time->tm_hour+14.4*tick_time->tm_min+0.24*tick_time->tm_sec+0.00024*(time_ms(NULL,NULL)+ms_clock_offset+ensure_change)) % 20736; }
  else if(settings.diurnal==1){ lull = (int)(144*tick_time->tm_hour+2.4*tick_time->tm_min+0.04*tick_time->tm_sec+0.00004*(time_ms(NULL,NULL)+ms_clock_offset+ensure_change)) % 3456; }
	else if(settings.diurnal==2){ lull = (int)(288*tick_time->tm_hour+4.8*tick_time->tm_min+0.08*tick_time->tm_sec+0.00008*(time_ms(NULL,NULL)+ms_clock_offset+ensure_change)) % 6912; }
	else if(settings.diurnal==4){ lull = (int)(432*tick_time->tm_hour+7.2*tick_time->tm_min+0.12*tick_time->tm_sec+0.00012*(time_ms(NULL,NULL)+ms_clock_offset+ensure_change)) % 10368; }
  else{ lull = (int)(216*tick_time->tm_hour+3.6*tick_time->tm_min+0.06*tick_time->tm_sec+0.00006*(time_ms(NULL,NULL)+ms_clock_offset+ensure_change)) % 5184; }
	
	// one decimal digit for diurnal, two digits otherwise

	if((settings.diurnal==0)|(settings.diurnal==5)){
		write_dozenal(lull/12,3,lead);
		write_dozenal(lull%12,1,trail);
		snprintf(buffer, sizeof(buffer), "%s.%s", lead, trail); 
	} else{
		write_dozenal(lull/144,2,lead);
		write_dozenal(lull%144,2,trail);
		snprintf(buffer, sizeof(buffer), "%s.%s", lead, trail);
	}
	
	text_layer_set_text(s_time_layer, buffer);
	
	if(lull==0){ refresh_date(); }
	  	
}

static void delay_to_lull(){

	int ms_wait = ms_to_next_lull();
	if((ms_wait<3)||(ms_wait>250)){ refresh_time(); }
	else{ app_timer_register(4*ms_wait/5,delay_to_lull,NULL); }

}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
	
	// read data back from phone using message keys
	
  Tuple *diurnal_t = dict_find(iter, MESSAGE_KEY_DIURNAL);
  Tuple *temperature_scale_t = dict_find(iter, MESSAGE_KEY_TEMPERATURE_SCALE);
	Tuple *wind_speed_format_t = dict_find(iter, MESSAGE_KEY_WIND_SPEED_FORMAT);
  Tuple *uv_format_t = dict_find(iter, MESSAGE_KEY_UV_FORMAT);
	Tuple *date_format_t = dict_find(iter, MESSAGE_KEY_DATE_FORMAT);	
	Tuple *temperature_t = dict_find(iter, MESSAGE_KEY_TEMPERATURE);
	Tuple *city_t = dict_find(iter, MESSAGE_KEY_CITY);
	Tuple *conditions_t = dict_find(iter, MESSAGE_KEY_CONDITIONS);
	Tuple *humidity_t = dict_find(iter, MESSAGE_KEY_HUMIDITY);
	Tuple *wind_kph_t = dict_find(iter, MESSAGE_KEY_WIND_KPH);
  Tuple *uv_t = dict_find(iter, MESSAGE_KEY_UV);
	Tuple *pressure_mb_t = dict_find(iter, MESSAGE_KEY_PRESSURE_MB);
	Tuple *pressure_format_t = dict_find(iter, MESSAGE_KEY_PRESSURE_FORMAT);
	Tuple *wind_degrees_t = dict_find(iter, MESSAGE_KEY_WIND_DEGREES);
	Tuple *wind_degrees_format_t = dict_find(iter, MESSAGE_KEY_WIND_DEGREES_FORMAT);
	Tuple *origin_t = dict_find(iter, MESSAGE_KEY_ORIGIN);
	Tuple *season_t = dict_find(iter, MESSAGE_KEY_SEASON);
	Tuple *hemisphere_t = dict_find(iter, MESSAGE_KEY_HEMISPHERE);
	Tuple *latitude_t = dict_find(iter, MESSAGE_KEY_LATITUDE);
	Tuple *api_key_t = dict_find(iter, MESSAGE_KEY_API_KEY);

	// multiplied temperature and wind_kph by 10 before passing from watch and uv
	// now convert back to real values
	
  if (temperature_t){ settings.temperature = 0.1*((float)temperature_t->value->int32);  }
	if (wind_kph_t){ settings.wind_kph = 0.1*((float)wind_kph_t->value->int32); }
  if (uv_t){ if(uv_t->value->int32>=0){ settings.uv = 0.1*((float)uv_t->value->int32); stale_uv=0; } else { stale_uv = 1; }}

	// redraw date if latitude shifts across equator
	
	if (latitude_t){ 
		float temp =  settings.latitude;
		settings.latitude = 0.000001*((float)latitude_t->value->int32);
		if(temp*settings.latitude<0){ refresh_date(); }
	}
	
  if (humidity_t){ settings.humidity = humidity_t->value->int32; }
  if (pressure_mb_t){ settings.pressure_mb = pressure_mb_t->value->int32; }
	//if (pressure_mb_t){ settings.pressure_mb = 0.01*((float)pressure_mb_t->value->int32); } // for inches
  if (wind_degrees_t){ settings.wind_degrees = wind_degrees_t->value->int32; }
		
  if (wind_degrees_format_t) { settings.wind_degrees_format = atoi(wind_degrees_format_t->value->cstring); }
	if (pressure_format_t) { settings.pressure_format = atoi(pressure_format_t->value->cstring); }
	if (temperature_scale_t) { settings.temperature_scale = atoi(temperature_scale_t->value->cstring); }
	if (wind_speed_format_t) { settings.wind_speed_format = atoi(wind_speed_format_t->value->cstring); }
	if (uv_format_t) { settings.uv_format = atoi(uv_format_t->value->cstring); }
	if (diurnal_t) { settings.diurnal = atoi(diurnal_t->value->cstring); }
	if (date_format_t) { settings.date_format = atoi(date_format_t->value->cstring); }
	if (origin_t) { settings.origin = atoi(origin_t->value->cstring); }
	if (season_t) { settings.season = atoi(season_t->value->cstring); }
	if (hemisphere_t) { settings.hemisphere = atoi(hemisphere_t->value->cstring); }
	if(api_key_t){ strcpy(settings.api_key,api_key_t->value->cstring); }
	
  // abbreviate city name

	if (city_t){ 
		snprintf(settings.city,sizeof(settings.city),"%s",city_t->value->cstring);
		snprintf(settings.city,sizeof(settings.city),"%s",replace_str(settings.city,"North ","N. "));
		snprintf(settings.city,sizeof(settings.city),"%s",replace_str(settings.city,"East ","E. "));
		snprintf(settings.city,sizeof(settings.city),"%s",replace_str(settings.city,"South ","S. "));
		snprintf(settings.city,sizeof(settings.city),"%s",replace_str(settings.city,"West ","W. "));
	}

	if (conditions_t){ 
		//https://www.wunderground.com/weather/api/d/docs?d=resources/phrase-glossary&MR=1
		snprintf(settings.conditions,sizeof(settings.conditions),"%s",conditions_t->value->cstring);
		snprintf(settings.conditions,sizeof(settings.conditions),"%s",replace_str(settings.conditions,"Light ","Lt "));
		snprintf(settings.conditions,sizeof(settings.conditions),"%s",replace_str(settings.conditions,"Heavy ","Hvy "));			
		snprintf(settings.conditions,sizeof(settings.conditions),"%s",replace_str(settings.conditions,"Thunderstorm","T-Storm"));	
		snprintf(settings.conditions,sizeof(settings.conditions),"%s",replace_str(settings.conditions,"Widespread","Wspr"));	
		snprintf(settings.conditions,sizeof(settings.conditions),"%s",replace_str(settings.conditions," and "," & "));	
		snprintf(settings.conditions,sizeof(settings.conditions),"%s",replace_str(settings.conditions," with "," w "));	
	}
	
	// write settings to persistent memory
		
	persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
	
	// redraw watchface

	if ((diurnal_t) || (origin_t)) { refresh_time(); }
	if ((date_format_t) || (origin_t) || (hemisphere_t) || (season_t)) { refresh_date(); }
	if (temperature_t || wind_kph_t || humidity_t || uv_t || conditions_t || city_t || temperature_scale_t || wind_speed_format_t || wind_degrees_t || pressure_format_t){ refresh_weather(); }
	if (api_key_t) { request_weather(); };
	
}

//static void tap_handler(AccelAxisType axis, int32_t direction){
	
	//APP_LOG(0,"Tap detected");
	
//}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){

	static int suppress_next=0;

	if(suppress_next < 1){
		int ms_wait = ms_to_next_lull();
		if(ms_wait<150){ app_timer_register(4*ms_wait/5,delay_to_lull,NULL); suppress_next = 3; }
		else if(ms_wait<1100){ app_timer_register(ms_wait-150,delay_to_lull,NULL); suppress_next = 3; }
	}
	else { suppress_next--; }
	
	if(tick_time->tm_sec==0){
		int minutes_since_midnight = tick_time->tm_hour*60+tick_time->tm_min;
		if(minutes_since_midnight%WEATHER_REFRESH_MINS==0){ request_weather(); }		
	}
	
}

static void main_window_load(Window *window) {

	Layer *window_layer = window_get_root_layer(window);
	
  //GRect bounds = layer_get_frame(window_layer);
	//s_scroll_layer = scroll_layer_create(bounds);
  //scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
	
  s_date_layer = text_layer_create(GRect(0,0,0,0));
  s_weekday_layer = text_layer_create(GRect(0,0,0,0));
  s_degrees_layer = text_layer_create(GRect(0,0,0,0));
  s_time_layer = text_layer_create(GRect(0,0,0,0));
  s_weather_layer = text_layer_create(GRect(0,0,0,0));
  s_conditions_layer = text_layer_create(GRect(0,0,0,0));
  s_city_layer = text_layer_create(GRect(0,0,0,0));
	
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VERDANA_NBOLD_36));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VERDANA_NBOLD_18));
  s_conditions_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VERDANA_RBOLD_18));
  
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorFromHEX(0x1200FF));
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  text_layer_set_background_color(s_weekday_layer, GColorClear);
  text_layer_set_text_color(s_weekday_layer, GColorFromHEX(0x1200FF));
  text_layer_set_font(s_weekday_layer, s_date_font);
  text_layer_set_text_alignment(s_weekday_layer, GTextAlignmentLeft);
  
  text_layer_set_background_color(s_degrees_layer, GColorClear);
  text_layer_set_text_color(s_degrees_layer, GColorBlack);
  text_layer_set_font(s_degrees_layer, s_date_font);
	text_layer_set_text_alignment(s_degrees_layer, GTextAlignmentLeft);
	
  //set right or centre: see above  
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorFromHEX(0x1200FF));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_font(s_weather_layer, s_date_font);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);

  text_layer_set_background_color(s_conditions_layer, GColorClear);
  text_layer_set_text_color(s_conditions_layer, GColorBulgarianRose);
  text_layer_set_font(s_conditions_layer, s_conditions_font);
  text_layer_set_text_alignment(s_conditions_layer, GTextAlignmentCenter);
  
  text_layer_set_background_color(s_city_layer, GColorClear);
  text_layer_set_text_color(s_city_layer, GColorBulgarianRose);
  text_layer_set_font(s_city_layer, s_conditions_font);
  text_layer_set_text_alignment(s_city_layer, GTextAlignmentCenter);

  	//scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, 200)); // max scroll height set here
	
	//scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_time_layer));
	//scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_date_layer));
	//scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_city_layer));
	//scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_weekday_layer));
  //[new]scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_degrees_layer));
	//scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_conditions_layer));
	//scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_weather_layer));
	
	layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_city_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_weekday_layer));
  //new
  layer_add_child(window_layer, text_layer_get_layer(s_degrees_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_conditions_layer));
	layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
	
  //layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));	
	
}

static void main_window_unload(Window *window) {
  
	text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
	text_layer_destroy(s_weekday_layer);
  //new
  text_layer_destroy(s_degrees_layer);
  text_layer_destroy(s_conditions_layer);
	text_layer_destroy(s_weather_layer);
	text_layer_destroy(s_city_layer);
  
  //scroll_layer_destroy(s_scroll_layer);

}

int main(void) {

	if (persist_exists(SETTINGS_KEY)) { persist_read_data(SETTINGS_KEY, &settings, sizeof(settings)); } 
	else {
		settings.origin = 0;
		settings.diurnal = 0;
		settings.hemisphere = 0;
		settings.latitude = 0;
		settings.season = 3;
		settings.temperature_scale = 0;
		settings.date_format = 3;
		settings.wind_speed_format = 0;
		settings.wind_degrees = 0;
    settings.pressure_format = 0;
		settings.uv_format = 0;
    settings.uv = -1;
		strcpy(settings.api_key,"");
	}
	
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {.load = main_window_load,.unload = main_window_unload,});
  window_stack_push(s_main_window, true);	

	refresh_time();
	refresh_date();
	
	/*
	int query = time_from_yyyymmdd(20161101);
	month_struct result;
	for(int i=0;i<1000;++i){
		result = get_dozenal_date(&WINTER,query);
		if(result.day==31){ APP_LOG(APP_LOG_LEVEL_INFO,"%d %d %d %d",i,result.year,result.month,result.day); }
		query = query+86400;
	}*/
	
	app_message_register_inbox_received(inbox_received_handler);
	app_message_open(128, 128);
	
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);	
	//accel_tap_service_subscribe(tap_handler);
	
  app_event_loop();
  
	window_destroy(s_main_window);
	
}
