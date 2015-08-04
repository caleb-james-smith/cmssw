def setPadPasMargin(pad, rightMargin=0.05):                                                                                   
  pad.SetFrameFillStyle(1001)                                                                                                 
  pad.SetTicks()                                                                                                              
  pad.SetTopMargin(0)                                                                                                         
  pad.SetFillColor(0)                                                                                                         
  leftMargin   = 0.16                                                                                                         
  topMargin    = 0.1                                                                                                          
  bottomMargin = 0.15                                                                                                         
  pad.SetLeftMargin(leftMargin)                                                                                               
  pad.SetRightMargin(rightMargin)                                                                                             
  pad.SetTopMargin(topMargin)                                                                                                 
  pad.SetBottomMargin(bottomMargin)                                                                                           


def setHistBasic(hist):
    hist.GetYaxis().SetLabelSize(0.045)
    hist.GetYaxis().SetTitleSize(0.055)
    hist.GetXaxis().SetLabelSize(0.045)
    hist.GetXaxis().SetTitleSize(0.055)
    hist.GetXaxis().SetNdivisions(506)
    hist.GetXaxis().SetTitleOffset(1.15)
    hist.GetXaxis().SetLabelFont(62)
    hist.GetYaxis().SetLabelFont(62)
    hist.GetXaxis().SetTitleFont(62)
    hist.GetYaxis().SetTitleFont(62)
    hist.GetXaxis().SetNdivisions(412,1)
    return 0

def setHist(hist, xtitle, ytitle, xrange, yrange, yoff, color, style=1):
    setHistBasic(hist)
    hist.GetYaxis().SetTitle(ytitle)
    hist.GetXaxis().SetTitle(xtitle)
    hist.SetLineColor(color)
    hist.SetLineStyle(style)
    hist.SetLineWidth(2)
    hist.GetYaxis().SetTitleOffset(yoff)                                                                               
    if yrange != 0:                                                                             
        hist.GetYaxis().SetRangeUser(yrange[0], yrange[1])
    if xrange != 0:
        hist.GetXaxis().SetRangeUser(xrange[0], xrange[1])
    return 0

def setHist2D(hist, xtitle, ytitle, ztitle, xrange, yrange, zrange, xoff, yoff, zoff):
    setHistBasic(hist)
    hist.GetYaxis().SetTitle(ytitle)
    hist.GetXaxis().SetTitle(xtitle)
    hist.GetZaxis().SetTitle(ztitle)
    hist.GetYaxis().SetTitleOffset(yoff)                                                                               
    hist.GetXaxis().SetTitleOffset(xoff)                                                                               
    hist.GetZaxis().SetTitleOffset(zoff)                                                                               
    if yrange != 0:                                                                             
        hist.GetYaxis().SetRangeUser(yrange[0], yrange[1])
    if xrange != 0:
        hist.GetXaxis().SetRangeUser(xrange[0], xrange[1])
    if zrange != 0:
        hist.GetZaxis().SetRangeUser(zrange[0], zrange[1])
    return 0

def setGraph(hist, xtitle, ytitle, xrange, yrange, yoff, color, mstyle, msize):
    hist.SetMarkerStyle(mstyle)
    hist.SetMarkerColor(color)
    hist.SetLineColor  (color)
    hist.SetFillColor  (0)
    hist.SetMarkerSize (msize)
    hist.GetYaxis().SetTitle(ytitle)
    hist.GetXaxis().SetTitle(xtitle)
    hist.GetYaxis().SetLabelSize(0.045)                                                                                
    hist.GetYaxis().SetTitleSize(0.055)                                                                                
    hist.GetYaxis().SetTitleOffset(yoff)                                                                               
    hist.GetXaxis().SetLabelSize(0.045)                                                                                
    hist.GetXaxis().SetTitleSize(0.055)                                                                                
    hist.GetXaxis().SetNdivisions(506)                                                                                 
    hist.GetXaxis().SetTitleOffset(1.15)                                                                               
    hist.GetXaxis().SetLabelFont(62)                                                                                   
    hist.GetYaxis().SetLabelFont(62)                                                                                   
    hist.GetXaxis().SetTitleFont(62)                                                                                   
    hist.GetYaxis().SetTitleFont(62)                                                                                   
    hist.GetXaxis().SetNdivisions(412,1)
    if yrange != 0:                                                                             
        hist.GetYaxis().SetRangeUser(yrange[0], yrange[1])
    if xrange != 0:
        hist.GetXaxis().SetRangeUser(xrange[0], xrange[1])
    return 0
    
def addHists(hist1, hist2, name):
    hfist3 = hist1.Clone(name)
    hist3.Add(hist2)
    return hist3

def getText(ip, ip2, E_base_phase=0):
    outText = []
    if ip == "05" : ipA = "0.5"
    else : ipA = ip
    if ip2 == "05": ipB = "0.5"
    else : ipB = ip2
    name = "Peak current = "
    if ipB == 0:
        outText.append(name+ipA+" #muA")
    else:
        outText.append(name+ipA+" #muA + "+ipB+" #muA")
    outText.append("E_{tot} = "+uA2gev[ip, "t"]+" GeV")
    if E_base_phase == "0":
        outText.append("not timed in")
    else:
        outText.append("timed in for "+uA2gev[E_base_phase, "t"]+" GeV")
        #outText.append("phase = "+str(out_phase[E_base_phase])+" ns")
    return outText


# Channel List
chanList = range(1,73)

calib = {}
for channum in chanList:
    calib[channum] = 1.

runList = [7902]

chanType = {}
for channum in chanList:
    chanType[channum,runList[0]] = "Channel "+str(channum)

#chanType[4 , 7522] = "20x20cm SCSN-81 (PdB)"
#chanType[5 , 7522] = "T3 2x10cm"
#chanType[6 , 7522] = "PTP 6 blue WLS"
#chanType[12, 7522] = "PET 2mm/3mm tiles"
#chanType[17, 7522] = "PEN 2 sigma (2mm)"
#chanType[18, 7522] = "HE 10x10cm SCSN-81 (JF)"
#chanType[23, 7522] = "T5 CSL 2x10cm"
#chanType[22, 7522] = "2x10cm SCSN-81"

edges = {}
for channum in chanList:
    edges[channum,runList[0]] = [-80.    , 80.,     -80. ,     80.]

#edges = {}         #     x-,  x+,        y-,      y+
#edges[4 , 7526] = [-80.    , 80.,     -80. ,     80.]  #"20x20cm SCSN-81 (PdB)"
#edges[5 , 7526] = [12.-80. , 12.,  12.-15. ,     12.]  #"mix RTV" 10x2cm
#edges[6 , 7526] = [20.-80. , 20.,  44.-80. ,     44.]  #"PET 10x10cm 2sigma"
#edges[12, 7526] = [20.-80. , 20.,  42.-70. ,     42.]  #"Eileen PTP 8x10cm"
#edges[17, 7526] = [15.-80. , 15.,  45.-80. ,     45.]  #"PEN 10x10cm sigma"
#edges[18, 7526] = [15.-80. , 15.,  45.-80. ,     45.]  #"HE 10x10cm SCSN-81 (JF)"
#edges[22, 7526] = [25.-80. , 25., -5.      , -5.+15.]  #"T6"
#edges[23, 7526] = [15.-80. , 15., -1.      , -1.+15.]  #"mix 7 LS"

# ped,   1pe,   2pe,    chi2  
pecal = {}
for channum in chanList:
    pecal[channum] = [13.33, 35.19, 57.06, 164.45]
