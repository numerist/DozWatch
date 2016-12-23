module.exports = [
	
   {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Day Start"
      },
			{
				"type": "select",
				"messageKey": "ORIGIN",
				"defaultValue": 0,
				"label": "",
				"options": [
					{ 
						"label": "midnight", 
						"value": 0
					},
					{ 
						"label": "1/4 day after midnight", 
						"value": 1
					},
					{ 
						"label": "1/3 day after midnight", 
						"value": 2
            },
					{ 
						"label": "1/2 day after midnight", 
						"value": 3
            },
					{ 
						"label": "1/6 day after midnight", 
						"value": 4
					}					
				]
			}		
    ]
  },
  
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Time"
      },
			{
				"type": "select",
				"messageKey": "DIURNAL",
				"defaultValue": 0,
				"label": "",
				"options": [
					{ 
						"label": "diurnal", 
						"value": 0
					},
					{ 
						"label": "semi-diurnal", 
						"value": 1
					},
					{ 
						"label": "phasic", 
						"value": 2
					},
					{ 
						"label": "shift", 
						"value": 3
          },
					{ 
						"label": "semi-shift", 
						"value": 4
          }					
				]
			}		
    ]
  },
	
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Hemisphere"
      },
			{
				"type": "select",
				"messageKey": "HEMISPHERE",
				"defaultValue": 0,
				"label": "",
				"options": [
					{ 
						"label": "northern", 
						"value": 0
					},
					{ 
						"label": "southern", 
						"value": 1
					},
					{ 
						"label": "auto-detect", 
						"value": 2
          }
				]
			}		
    ]
  },
	
	{
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Season Start (Holocene only)"
      },
			{
				"type": "select",
				"messageKey": "SEASON",
				"defaultValue": 3,
				"label": "",
				"options": [
					{ 
						"label": "spring", 
						"value": 0
					},
					{ 
						"label": "summer", 
						"value": 1
					},
					{ 
						"label": "autumn", 
						"value": 2
            },
					{ 
						"label": "winter", 
						"value": 3
            }
				]
			}		
    ]
  },
	
	{
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Date"
      },
			{
				"type": "select",
				"messageKey": "DATE_FORMAT",
				"defaultValue": 3,
				"label": "",
				"options": [
					{ 
						"label": "yyyy-mm-dd",
						"value": 0
					},
					{ 
						"label": "m/d/yyyy", 
						"value": 1
					},
					{ 
						"label": "d.m.yyyy", 
						"value": 2
					},
					{ 
						"label": "holocene seasonal A", 
						"value": 3
					},
					{ 
						"label": "holocene seasonal B", 
						"value": 4
					},
					{ 
						"label": "holocene seasonal cardinal", 
						"value": 5
					}/*,
					{ 
						"label": "Holocene seasonal cardinal B", 
						"value": 6
					}		*/								
				]
			}		
    ]
  },
		
	{
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Temperature"
      },
			{
				"type": "select",
				"messageKey": "TEMPERATURE_SCALE",
				"defaultValue": 0,
				"label": "",
				"options": [
					{ 
						"label": "stadigrees crystallic", 
						"value": 0
					},
					{ 
						"label": "stadigrees familiar", 
						"value": 1
					},
					{ 
						"label": "tregrees", 
						"value": 2
					},
					{ 
						"label": "°G", 
						"value": 3
          },
					{  
            "label": "°C", 
						"value": 4
          },
					{  
            "label": "°F", 
						"value": 5
					}
				]
			}		
    ]
  },

  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Air Pressure"
      },
			{
				"type": "select",
				"messageKey": "PRESSURE_FORMAT",
				"defaultValue": 0,
				"label": "",
				"options": [
					{ 
						"label": "pressurels",
						"value": 0
	        },
					{ 
						"label": "unqualengthels Hg",
						"value": 1
          },
					{ 
						"label": "unquaPrems",
						"value": 2
          },
					{ 
						"label": "biquaGrafuts Hg",
						"value": 3
          },
          { 
						"label": "millibars",
						"value": 4  
          }
				]
			}		
    ]
  },
	
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "UV Index"
      },
			{
				"type": "select",
				"messageKey": "UV_FORMAT",
				"defaultValue": 0,
				"label": "",
				"options": [
					{ 
						"label": "quadciaintensitels",
						"value": 0
					},
					{ 
						"label": "hexciaPenz",
						"value": 1
					},
					{ 
						"label": "SI intensity",
						"value": 2
					}	
				]
			}		
    ]
  },
	
	{
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Wind Speed"
      },
			{
				"type": "select",
				"messageKey": "WIND_SPEED_FORMAT",
				"defaultValue": 0,
				"label": "",
				"options": [
					{ 
						"label": "velocitels",
						"value": 0
					},
					{ 
						"label": "unquaVlos", 
						"value": 1
					},
					{ 
					  "label": "km/h", 
						"value": 2
	        },
					{
            "label": "mph", 
						"value": 3
          }
				]
			}		
    ]
  },

  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Wind Direction"
      },
			{
				"type": "select",
				"messageKey": "WIND_DEGREES_FORMAT",
				"defaultValue": 0,
				"label": "",
				"options": [
					{ 
						"label": "biciaturns",
						"value": 0
					},
					{ 
						"label": "unciaPis", 
						"value": 1
          },
					{ 
						"label": "degrees", 
						"value": 2
          }
				]
			}		
    ]
  },
	
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
	
];
