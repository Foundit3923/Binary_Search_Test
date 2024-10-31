


void makeFuncRanges(){

}

void buildIndex(){
    
}

int main(){
    int line_count = 0;
  char line[58];
  int r_count = 0;
  char* functions_filepath = strdup(cwd);
  strcat(functions_filepath, function_filename);
  if(( Functions = fopen(functions_filepath, "r")) == NULL){
    function_file_exists = -1;
  }
  else{
    while (fgets(line, 58, Functions))
    {
      line_count++;
    }
  }
  
  int func_ranges[line_count][2];
  fseek(Functions, 0, SEEK_SET);
  fgets(line, 58, Functions);
  while (fgets(line, 58, Functions))
  {
      char* tmp = strdup(line);
      func_ranges[r_count][0] = (int)strtol(getfield(tmp, 1), NULL, 0);
      tmp = strdup(line);
      func_ranges[r_count][1] = (int)strtol(getfield(tmp, 2), NULL, 0);
      r_count++;
      free(tmp);
  }
}