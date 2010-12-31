function pcd = load_pcd(filename)
% pcd = load_pcd(filename)


f = fopen(filename);

columns = {};

% read header
while 1
   s = fgets(f);
   [t s] = strtok(s);
   if strcmp(t, 'COLUMNS') || strcmp(t, 'FIELDS')
       
      % convert new fields names to old field names
      s = strrep(s, 'normal_x', 'nx');
      s = strrep(s, 'normal_y', 'ny');
      s = strrep(s, 'normal_z', 'nz');
      s = strrep(s, 'principal_curvature_x', 'pcx');
      s = strrep(s, 'principal_curvature_y', 'pcy');
      s = strrep(s, 'principal_curvature_z', 'pcz');
      s = strrep(s, 'fpfh', ['f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 ' ...
                             'f15 f16 f17 f18 f19 f20 f21 f22 f23 f24 f25 f26 f27 f28 f29 f30 f31 f32 f33']);
      
      i = 0;
      s = strtrim(s);
      while ~isempty(s)
         i = i+1;
         [t s] = strtok(s);
         columns{i} = t;
      end
   elseif strcmp(t, 'DATA')
      [t s] = strtok(s);
      if ~strcmp(t, 'ascii')
         fprintf('Error: %s is not an ASCII file!\n', filename);
         fclose(f);
         return;
      end
      break;
   end
end

% read data
data = fscanf(f, '%f', [length(columns) inf])';

fclose(f);

pcd = populate_pcd_fields(columns, data);

