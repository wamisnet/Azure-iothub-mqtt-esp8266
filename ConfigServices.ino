
void initCloudConfig(String cs,const char *geo){
  cloud.geo = geo;
  cloud.host = GetStringValue(splitStringByIndex(splitStringByIndex(cs, ';', 0), '=', 1));
  cloud.id = GetStringValue(splitStringByIndex(splitStringByIndex(cs, ';', 1), '=', 1));
  cloud.key = (char*)GetStringValue(splitStringByIndex(splitStringByIndex(cs, ';', 2), '=', 1));
}


