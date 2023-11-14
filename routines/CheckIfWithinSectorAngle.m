function Flag = CheckIfWithinSectorAngle(LB, UB, Angle)

Range = [LB UB];
if (Range(2)-Range(1)) >= 0
   
    Flag = (Angle >= Range(1)) && (Angle <= Range(2) );
    
elseif (Range(2)-Range(1)) < 0

    Flag = (Angle >= Range(1)) || (Angle <= Range(2) );
    
end
    
end