def draw_boxes(colors,results,frame):
    #arduino = serial.Serial('COM3',9600)
    json_file = []
    for (color, result) in zip (colors, results):
        #Convert confidence level to int from float 
        json_temp = dict(result)
        json_conf = json_temp['confidence']
        json_conf = int(round(json_conf*100))
        json_temp['confidence'] = json_conf
        #till here
        json_file.append(json_temp)
        tl = (result['topleft']['x'], result['topleft']['y'])
        br = (result['bottomright']['x'], result['bottomright']['y'])
        corX = (result['topleft']['x'] + result['bottomright']['x']) /2
        corY = (result['topleft']['y'] + result['bottomright']['y']) /2
        centroids = (corX,corY)
        #centroids = int(centroids)
        
        print (centroids)

        confidence = result['confidence']
        label = result['label']
        #print('{},{},{},{}\n'.format(result['topleft']['x'], result['topleft']['y'],result['bottomright']['x'], result['bottomright']['y']))
        text = '{}: {:.1f}%'.format(label,confidence*100 )
        frame = cv2.rectangle(frame, tl, br, color, 5)
        frame = cv2.putText(frame, text, tl, cv2.FONT_HERSHEY_COMPLEX, 1, (0, 0, 0), 2)
     
    cv2.imshow ("predicted",frame)
    with open('data.json','w') as outfile:
        json.dump(json_file,outfile)