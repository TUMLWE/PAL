function txtout = addfield(inputtype, keyword, OpeningString, ClosingString, txt, addNewLine)

%  inputtype == 1; it inserts the fields before the keyword
%  inputtype == 0; it inserts the fields after the keyword

if addNewLine
    newstring = [newline OpeningString newline ClosingString newline newline];
else
    newstring = OpeningString;
end

k1 = strfind(txt, keyword);

if inputtype == 1

    if isempty(k1)
        error('create_matlabcodegen_fields: cound not find keyword " %s ", please check the reference file', keyword)
    end

    txtout = [txt(1:k1-1) newstring txt(k1:end)];

elseif inputtype == 0

    if isempty(k1)
        error('create_matlabcodegen_fields: cound not find keyword " %s ", please check the reference file', keyword)
    end


    k2 = k1 + length(keyword);
    txtout = [txt(1:k2) newstring txt(k2:end)];

elseif inputtype == 3  % regexp, match before idx1

    [idx1, idx2] = regexp(txt, keyword);

%          ii = 1
%         subtext = txt(idx1(ii) : idx2(ii))

    if isempty(idx1)
        error('Could not match pattern "%s": please check reference file', keyword)
    end

    txtout = [txt(1:idx1-1) newstring txt(idx1:end)];

elseif inputtype == 4  % regexp, match after idx2

    [idx1, idx2] = regexp(txt, keyword);

    if isempty(idx2)
        error('Could not match pattern "%s": please check reference file', keyword)
    end

    %         ii = 1
    %         subtext = txt(idx1(ii) : idx2(ii))

    txtout = [txt(1:idx2) newstring txt(idx2+1:end)];


elseif inputtype == 5  % substitute line

    %     keyword = ['\/\* default task cycle time in ms \(\-\>Task\_CfgRead\) \*\/']
    keyword = ['\n* *[\w\.]*(?=,\s*' keyword ')'];
    [idx1, idx2] = regexp(txt, keyword);

    newstring = [newline OpeningString];

    if isempty(idx1)
        error('Could not match pattern "%s": please check reference file', keyword)
    end

    txtout = [txt(1:idx1-1) newstring txt(idx2+1:end)];

    %         ii = 1
    %         subtext = txt(idx1(ii) : idx2(ii))


end
